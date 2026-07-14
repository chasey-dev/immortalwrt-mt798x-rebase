/*
 * Copyright (C) 2025  chasey-dev <ellenyoung0912@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
'use strict';

import { defs } from 'mtwifi.defaults';
import { set_indexed_value } from 'datconf';

// ==========================================
// Helper Functions
// ==========================================

/**
 * Convert an explicitly configured UCI boolean to a DAT token.
 *
 * Null means the UCI option is absent and must stay absent for callers that
 * want to preserve a DAT default.
 *
 * @param {any} val - UCI boolean-like value.
 * @returns {string|null} "1", "0", or null for absent input.
 */
function strict_bool(val) {
	if (val == null) return null;
	if (val === true || val == "1" || val == 1) return "1";
	return "0";
}

const PMF_REQUIRED_AUTH_MODES = [ "OWE", "WPA3PSK" ];

/**
 * Calculate the DAT PMF mode from AuthMode and UCI ieee80211w.
 *
 * Return values match DAT MFPR/MFPC decisions: 2 required, 1 optional, 0 off.
 *
 * @param {string} authmode - DAT AuthMode token.
 * @param {number} ieee80211w - Typed UCI PMF value.
 * @returns {number} 2, 1, or 0.
 */
function calc_pmf_mode(authmode, ieee80211w) {
	if (authmode in PMF_REQUIRED_AUTH_MODES)
		return 2;

	if (authmode == "WPA2PSKMIXWPA3PSK")
		return 1;

	return ieee80211w ?? 0;
}

/**
 * Mirror the mainline WPA-PSK-SHA256 rule into the DAT profile.
 *
 * @param {string} authmode - DAT AuthMode token.
 * @param {number} pmf_mode - Effective PMF mode.
 * @returns {string} DAT boolean token.
 */
function calc_pmf_sha256(authmode, pmf_mode) {
	return (pmf_mode != 0 && authmode in [ "WPA2PSK", "WPA2PSKMIXWPA3PSK" ]) ?
		"1" : "0";
}

/**
 * Extract the numeric VIF suffix used by indexed DAT AP values.
 *
 * Examples: ra0 -> 0, rax1 -> 1.
 *
 * @param {string} ifname - mtwifi VIF name.
 * @returns {number} Numeric VIF index.
 */
function get_vif_idx(ifname) {
	// The handler assigns this name; a broken contract must not alias slot 0.
	return int(match(ifname, /[0-9]+$/)[0]);
}

/**
 * Calculate the DAT WirelessMode value from UCI band and htmode.
 *
 * Reference values:
 *   9 = N/AC Mixed; 15 = VHT/N Mixed
 *   16/17/18 = AX 2g/5g/6g
 *   22/23/24 = BE 2g/5g/6g
 *
 * @param {string} band - UCI band: 2g, 5g, or 6g.
 * @param {string} htmode - UCI htmode.
 * @returns {number} DAT WirelessMode integer.
 */
function calc_wireless_mode(band, htmode) {
	let is_ax = index(htmode, "HE") == 0;
	let is_be = index(htmode, "EHT") == 0;

	switch (band) {
	case "2g":
		return is_be ? 22 : (is_ax ? 16 : 9);
	case "5g":
		return is_be ? 23 : (is_ax ? 17 : 15);
	case "6g":
		return is_be ? 24 : 18;
	default:
		return 9; // Fallback
	}
}

/**
 * Calculate DAT bandwidth fields from UCI htmode.
 *
 * @param {string} htmode - UCI htmode.
 * @param {any} noscan - UCI noscan/HT coexistence value.
 * @returns {Object} DAT bandwidth fields including HT_BW, VHT_BW, EHT_ApBw.
 */
function calc_bandwidth(htmode, noscan) {
	let is_eht = index(htmode, "EHT") == 0;
	let bw_match = match(htmode, /\d+/);
	let width = bw_match ? bw_match[0] : "20";

	/*
	 * Return shape:
	 * - HT_BW/VHT_BW keep legacy bandwidth values.
	 * - EHT_ApBw is raised only for EHT modes; 0 clears stale EHT width.
	 * - HT_EXTCHA is emitted only for explicit EHT320/EHT320-2.
	 * - HT_BSSCoexistence follows noscan for 40MHz.
	 */
	let res = {
		"HT_BW": "0",
		"VHT_BW": "0",
		"HT_BSSCoexistence": "1",
		"EHT_ApBw": "0"
	};
	switch (width) {
	case "40":
		res.HT_BW = "1";
		if (is_eht) res.EHT_ApBw = "1";
		res.HT_BSSCoexistence = noscan ? "0" : "1";
		break;
	case "80":
		res.HT_BW = "1";
		res.VHT_BW = "1";
		if (is_eht) res.EHT_ApBw = "2";
		break;
	case "160":
		res.HT_BW = "1";
		res.VHT_BW = "2";
		if (is_eht) res.EHT_ApBw = "3";
		break;
	case "320":
		res.HT_BW = "1";
		res.VHT_BW = "2";
		if (is_eht) {
			res.EHT_ApBw = "4";
			if (htmode == "EHT320") {
				res.HT_EXTCHA = "0";
			} else if (htmode == "EHT320-2") {
				res.HT_EXTCHA = "1";
			}
		}
		break;
	}

	// for 20MHz, keep default 0, 0
	return res;
}

// ==========================================
// UCI config => DAT config
// ==========================================

/**
 * Convert one netifd/UCI radio payload to DAT key/value updates.
 *
 * The result only represents the current band profile. AP values are encoded as
 * indexed DAT tokens, while ApCli values target the single supported ApCli slot.
 * Interface capacity and MLO topology are admitted before this payload reaches
 * conversion.
 * A key in values is overwritten, a key in unset is deleted, and a key in
 * neither collection keeps its existing DAT value.
 *
 * @param {Object} uci_cfg - netifd wireless payload for one radio.
 * @returns {Object} DAT update with values to merge and keys to unset.
 */
export function convert(uci_cfg) {
	let dat = {};
	let conf = uci_cfg.config;
	let ifaces = uci_cfg.interfaces;

	// BssidNum is the MBSSID slot capacity used by cfg80211 add_virtual_intf.
	// Keep it stable so hostapd can add/remove ext BSS without module reload.
	let bssid_count = defs.MAX_MBSSID;
	dat.BssidNum = bssid_count;

	// ------------------------------------------
	// global radio config in current DEVICE
	// ------------------------------------------

	// WirelessMode + BandWidth
	let bw_res = calc_bandwidth(conf.htmode, conf.noscan);
	dat.HT_BW = bw_res.HT_BW;
	dat.VHT_BW = bw_res.VHT_BW;
	dat.EHT_ApBw = bw_res.EHT_ApBw;
	if (bw_res.HT_EXTCHA != null) dat.HT_EXTCHA = bw_res.HT_EXTCHA;
	dat.HT_BSSCoexistence = bw_res.HT_BSSCoexistence;

	// calculate wireless mode
	let wmode_int = calc_wireless_mode(conf.band, conf.htmode);
	dat.WirelessMode = wmode_int;
	let is_be = index(conf.htmode, "EHT") == 0;
	let is_ax = index(conf.htmode, "HE") == 0;
	// MldGroup 0 is remapped to a driver-allocated single-link MLD group
	if (is_be) {
		for (let i = 0; i < bssid_count; i++)
			dat.MldGroup = set_indexed_value(dat.MldGroup || "", i, "0");
	}

	// Channel, check auto or not
	if (conf.channel == "auto") {
		dat.AutoChannelSelect = "3";
		dat.Channel = "0";
	} else {
		dat.AutoChannelSelect = "0";
		dat.Channel = conf.channel;
	}

	// CountryRegion code
	if (conf.country && length(conf.country) == 2) {
		dat.CountryCode = conf.country;
		let regions = defs.COUNTRY_REGIONS[conf.country] || [1, 0];
		if (conf.band == "2g") {
			dat.CountryRegion = regions[0];
		} else {
			dat.CountryRegionABand = regions[1];
		}
	}

	// TXPower
	let txp = int(conf.txpower);
	if (txp && txp < 100) {
		dat.PERCENTAGEenable = "1";
		dat.TxPower = txp;
	} else {
		dat.PERCENTAGEenable = "0";
		dat.TxPower = "100";
	}

	// Beamforming
	if (conf.mu_beamformer) {
		dat.ETxBfEnCond = "1";
		dat.ITxBfEn = "0";
		dat.MUTxRxEnable = "1";
	} else {
		dat.ETxBfEnCond = "0";
		dat.MUTxRxEnable = "0";
		dat.ITxBfEn = "0";
	}

	// TWT, ALWAYS set to 0 for NON AX/BE devices
	dat.TWTSupport = ((is_ax || is_be) && conf.twt) ? conf.twt : "0";

	// wifi-device cfgs stored in the current band profile
	for (let uci_key, v in defs.DEVICE_CFGS) {
		let dat_key = v[0];
		let def_val = v[1];
		dat[dat_key] = conf[uci_key] || def_val;
	}

	// ------------------------------------------
	// ApCli (Client/STA)
	// ------------------------------------------

	for (let k, v in defs.APCLI_CFGS) dat[k] = v;

	for (let k, iface in ifaces) {
		let c = iface.config;
		if (c.mode != "sta")
			continue;

		/* skip null tokens */
		let set_token = function(key, val) {
			if (val != null)
				dat[key] = val;
		};

		if (conf.mu_beamformer)
			dat.MUTxRxEnable = "3";

		dat.ApCliEnable = "1";
		dat.ApCliSsid = c.ssid;
		dat.ApCliBssid = c.bssid;
		if (c.macaddr)
			dat.ApcliMacAddress = c.macaddr;
		dat.ApCliWPAPSK = c.key;
		dat.ApCliWirelessMode = wmode_int;

		set_token("ApCliMuMimoDlEnable", strict_bool(c.mumimo_dl));
		set_token("ApCliMuMimoUlEnable", strict_bool(c.mumimo_ul));
		set_token("ApCliMuOfdmaDlEnable", strict_bool(c.ofdma_dl));
		set_token("ApCliMuOfdmaUlEnable", strict_bool(c.ofdma_ul));
		set_token("ApCliPweMethod", defs.SAE_PWE_2_DAT[c.sae_pwe]);

		// only one sae_group for ApCli
		set_token("ApCliSAEGroup", c.sae_groups?.[0]);

		// uci encryption mode => DAT cfg
		let enc_info = defs.ENC_2_COMMON_DAT[c.encryption];
		let authmode = enc_info[0];
		let pmf_mode = calc_pmf_mode(authmode, c.ieee80211w);

		dat.ApCliAuthMode = authmode;
		dat.ApCliEncrypType = enc_info[1];
		dat.ApCliPMFSHA256 = calc_pmf_sha256(authmode, pmf_mode);

		// ApCli PMF
		if (pmf_mode == 2) {
			dat.ApCliPMFMFPC = "1";
			dat.ApCliPMFMFPR = "1";
		} else if (pmf_mode == 1) {
			dat.ApCliPMFMFPC = "1";
			dat.ApCliPMFMFPR = "0";
		} else {
			dat.ApCliPMFMFPC = "0";
			dat.ApCliPMFMFPR = "0";
		}
	}

	// ------------------------------------------
	// vif => AP setting, set defaults first
	// ------------------------------------------

	// in DAT config, driver expect setting patterns like "1;1;0" for cfgs except suffix-key cfgs
	// we set default strings for each Bssid

	// set default AP cfgs
	for (let k, v in defs.AP_CFGS) {
		let default_str = "";
		for (let i = 0; i < bssid_count; i++) {
			default_str = set_indexed_value(default_str, i, v);
		}
		dat[k] = default_str;
	}

	// Clear driver ACL state; hostapd owns UCI macfilter/maclist.
	for (let k, v in defs.AP_ACL) {
		for (let i = 0; i < defs.MAX_MBSSID; i++) {
			dat[`${k}${i}`] = v;
		}
	}

	// clear suffix-key cfgs
	// like SSIDx, WPAPSKx
	// refer to schema/mtwifi/dat-defs.json
	for (let k, v in defs.AP_CFGS_IDX) {
		for (let i = 1; i <= defs.MAX_MBSSID; i++) {
			dat[`${k}${i}`] = "";
		}
	}

	// ------------------------------------------
	// vif => AP setting, set AP setting for every vif
	// ------------------------------------------

	for (let k, iface in ifaces) {
		let c = iface.config;
		if (c.mode != "ap") continue;
		// get vif index from name
		let vif_idx = get_vif_idx(iface.mtwifi_ifname);

		// Suffix Key Setting:
		// here SSIDx is 1-based, so SSIDx = vif_idx + 1
		// ra0 (0) -> SSID1
		// ra1 (1) -> SSID2
		let suffix_key_idx = vif_idx + 1;

		// Suffix-key cfg setting helper function
		// only set when UCI cfg exists, keep DAT default in other cases
		// like SSIDx, WPAPSKx, they are filled with single values
		let set_suffix = function(key, val) {
			if (val != null) {
				dat[`${key}${suffix_key_idx}`] = val;
			}
		};

		// common Token cfg setting helper function
		// only set when UCI cfg exists, keep DAT default in other cases
		// sets dat[key] value of current k (also ap_idx)
		// e.g. 12;17;26, sets 17 when k = 1, also now ap_idx = 1
		let set_token = function(key, val) {
			if (val != null) {
				dat[key] = set_indexed_value(dat[key], vif_idx, val);
			}
		};

		// set suffix-key settings
		set_suffix("SSID", c.ssid);
		set_suffix("WPAPSK", c.key);
		if (c.macaddr)
			dat[(vif_idx == 0) ? "MacAddress" : `MacAddress${vif_idx}`] = c.macaddr;

		// base cfgs
		set_token("WirelessMode", wmode_int); // here WirelessMode is set twice, we keep it for safety
		set_token("NoForwarding", strict_bool(c.isolate));
		set_token("HideSSID", strict_bool(c.hidden));
		set_token("WmmCapable", strict_bool(c.wmm));
		set_token("APSDCapable", strict_bool(c.uapsd));
		set_token("RTSThreshold", c.rts);
		set_token("FragThreshold", c.frag);
		set_token("DtimPeriod", c.dtim_period);
		set_token("RekeyInterval", c.wpa_group_rekey);

		// 802.11k/v/r
		set_token("RRMEnable", strict_bool(c.ieee80211k));
		set_token("FtSupport", strict_bool(c.ieee80211r));

		// HT settings
		set_token("HT_AMSDU", strict_bool(c.amsdu));
		set_token("HT_AutoBA", strict_bool(c.autoba));

		if (c.mlo) {
			/*
			 * Input admission guarantees EHT, and the handler validates the
			 * mainline ap-mldN identity before conversion.
			 */
			let m = match(c.ifname, /^ap-mld([0-9]+)$/);
			set_token("MldGroup", int(m[1]) + 1);
		}

		// MU-MIMO / OFDMA
		set_token("MuMimoDlEnable", strict_bool(c.mumimo_dl));
		set_token("MuMimoUlEnable", strict_bool(c.mumimo_ul));
		set_token("MuOfdmaDlEnable", strict_bool(c.ofdma_dl));
		set_token("MuOfdmaUlEnable", strict_bool(c.ofdma_ul));

		set_token("PweMethod", defs.SAE_PWE_2_DAT[c.sae_pwe]);

		// AuthMode + EncrypType
		let enc_def = (c.mlo && defs.ENC_2_MLO_AP_DAT[c.encryption]) ||
			defs.ENC_2_COMMON_DAT[c.encryption];
		let authmode = enc_def[0];
		let pmf_mode = calc_pmf_mode(authmode, c.ieee80211w);

		set_token("AuthMode", authmode);
		set_token("EncrypType", enc_def[1]);
		set_token("PMFSHA256", calc_pmf_sha256(authmode, pmf_mode));

		// AP PMF
		if (pmf_mode == 2) {
			set_token("PMFMFPC", "1");
			set_token("PMFMFPR", "1");
		} else if (pmf_mode == 1) {
			set_token("PMFMFPC", "1");
			set_token("PMFMFPR", "0");
		} else {
			// NOTE:
			// in AP_CFGS defaults, they were set to 0
			// override to 0 if there are special cases
		}

		// RekeyMethod
		if (authmode != "OPEN" && authmode != "OWE") {
			set_token("RekeyMethod", "TIME");
		}
	}

	/*
	 * Mac address overrides are presence-sensitive in the driver profile parser:
	 * an absent key selects the default/derived address, while an empty value is
	 * parsed as an explicit address and rejected. Revoke every unused override;
	 * a valid explicit value already present in dat is kept out of this list.
	 */
	let unset = [];
	for (let i = 0; i < defs.MAX_MBSSID; i++) {
		let key = i ? `MacAddress${i}` : "MacAddress";
		if (!exists(dat, key))
			push(unset, key);
	}
	for (let i = 0; i < defs.MAX_APCLI_NUM; i++) {
		let key = i ? `ApcliMacAddress${i}` : "ApcliMacAddress";
		if (!exists(dat, key))
			push(unset, key);
	}

	return { values: dat, unset };
};
