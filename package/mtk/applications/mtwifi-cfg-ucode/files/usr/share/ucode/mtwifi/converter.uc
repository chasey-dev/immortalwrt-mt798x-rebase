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

function to_str(val) {
	if (val == null) return "";
	return val + "";
}

// UCI bool => 0 / 1
// for null inputs that not set in UCI, return null
function strict_bool(val) {
	if (val == null) return null;
	if (val === true || val == "1" || val == 1) return "1";
	return "0";
}

// extract idx from vif names
// e.g. ra0 -> 0, rax1 -> 1
function get_vif_idx(ifname) {
	if (!ifname) return 0;
	let m = match(ifname, /[0-9]+$/);
	return m ? int(m[0]) : 0;
}

// calculate WirelessMode in DAT config
// Ref: 
// 9  = N/AC Mixed, 
// 15 = VHT/N Mixed,
// 16 = AX_24G,
// 17 = AX_5G,
// 18 = AX_6G
function calc_wireless_mode(band, htmode) {
	// check htmode key
	let is_ax = (index(htmode, "HE") == 0); // HE20, HE40, HE80...
	
	if (band == "2g") {
		return is_ax ? 16 : 9;
	} else if (band == "5g") {
		return is_ax ? 17 : 15;
	} else if (band == "6g") {
		return 18;
	}
	return 9; // Fallback
}

// calculate bandwidth (HT_BW, VHT_BW)
function calc_bandwidth(htmode, noscan) {
	let bw_match = match(htmode, /\d+/);
	let width = bw_match ? bw_match[0] : "20";
	
	let res = { 
		"HT_BW": "0", 
		"VHT_BW": "0", 
		"HT_BSSCoexistence": "1" 
	};

	if (width == "40") {
		res.HT_BW = "1";
		res.HT_BSSCoexistence = (noscan == "1") ? "0" : "1";
	} else if (width == "80") {
		res.HT_BW = "1";
		res.VHT_BW = "1";
	} else if (width == "160") {
		res.HT_BW = "1";
		res.VHT_BW = "2";
	}
	// for 20MHz, keep default 0, 0
	return res;
}

// ==========================================
// UCI config => DAT config
// ==========================================

export function convert(uci_cfg) {
	let dat = {};
	let conf = uci_cfg.config || {}; // uci device config
	let ifaces = uci_cfg.interfaces || {};

	// ------------------------------------------
	// count vifs in UCI config
	// ------------------------------------------
    
    let max_ap_idx = 0; // max AP index, 0 based for uci cfg
	let ap_count = 0;   // total AP count
	let has_apcli = false; // ApCli flag, ApCli appears in per DEVICE

	for (let k, iface in ifaces) {
		let c = iface.config;
		if (c.mode == "ap") {
			ap_count++;
			let idx = get_vif_idx(iface.mtwifi_ifname);
			if (idx > max_ap_idx) max_ap_idx = idx;
		} else if (c.mode == "sta") {
			has_apcli = true;
		}
	}

	// a little tricky here:
	// BssidNum is how many SSIDs enabled, we assume that vifs were initialized by BssidNum
	// so we set BssidNum = max ap index + 1
	let bssid_count = (ap_count > 0) ? (max_ap_idx + 1) : 1;
	dat.BssidNum = to_str(bssid_count);

    // ------------------------------------------
	// global radio config in current DEVICE 
	// ------------------------------------------
	
	// WirelessMode + BandWidth
	let bw_res = calc_bandwidth(conf.htmode, conf.noscan);
	dat.HT_BW = bw_res.HT_BW;
	dat.VHT_BW = bw_res.VHT_BW;
	dat.HT_BSSCoexistence = bw_res.HT_BSSCoexistence;
	
	// calculate wireless mode
	let wmode_int = calc_wireless_mode(conf.band, conf.htmode);
	dat.WirelessMode = to_str(wmode_int);
	let is_ax = (wmode_int >= 16); // for TWT checking, etc.

	// Channel, check auto or not
	if (conf.channel == "auto") {
		dat.AutoChannelSelect = "3";
		dat.Channel = "0";
	} else {
		dat.AutoChannelSelect = "0";
		dat.Channel = to_str(conf.channel);
	}

	// CountryRegion code
	if (conf.country && length(conf.country) == 2) {
		dat.CountryCode = conf.country;
		let regions = defs.COUNTRY_REGIONS[conf.country] || [1, 0];
		if (conf.band == "2g") {
			dat.CountryRegion = to_str(regions[0]);
		} else {
			dat.CountryRegionABand = to_str(regions[1]);
		}
	}

	// TXPower
	let txp = int(conf.txpower);
	if (txp && txp < 100) {
		dat.PERCENTAGEenable = "1";
		dat.TxPower = to_str(txp);
	} else {
		dat.PERCENTAGEenable = "0";
		dat.TxPower = "100";
	}

	// Beamforming
	if (conf.mu_beamformer) {
		dat.ETxBfEnCond = "1";
		dat.ITxBfEn = "0";
		// MUTxRxEnable set to 3 if has an apcli
		dat.MUTxRxEnable = has_apcli ? "3" : "1";
	} else {
		dat.ETxBfEnCond = "0";
		dat.MUTxRxEnable = "0";
		dat.ITxBfEn = "0";
	}

    // TWT, ALWAYS set to 0 for NON AX devices
	dat.TWTSupport = (is_ax && conf.twt) ? to_str(conf.twt) : "0";

	// chip-wide cfgs
	// for DBDC cards, only set for main DEVICE
	// TODO: need to handle logic for non-DBDC cards
	if (conf.dbdc_main) {
		for (let k, v in defs.CHIP_CFGS) {
			let uci_key = k;
			let dat_key = v[0];
			let def_val = v[1];
			// priority: uci_key, default value
			dat[dat_key] = to_str(conf[uci_key] || def_val);
		}
	}

	// ------------------------------------------
	// APCLI (Client/STA)
	// ------------------------------------------
	
	// set ApCliEnable to 0 first
	dat.ApCliEnable = "0";

	// clear old ApCli settings
	let apcli_reset_keys = ["ApCliSsid", "ApCliBssid", "ApCliAuthMode", "ApCliEncrypType", "ApCliWPAPSK"];
	for (let k in apcli_reset_keys) dat[k] = "";

	if (has_apcli) {
		for (let k, iface in ifaces) {
			let c = ifaces[k].config;
			if (c.mode == "sta") {
				dat.ApCliEnable = c.disabled ? "0" : "1";
				dat.ApCliSsid = to_str(c.ssid);
				dat.ApCliBssid = to_str(c.bssid);
				dat.ApCliWPAPSK = to_str(c.key);

				// uci encryption mode => DAT cfg 
				let enc_info = defs.ENC_2_DAT[c.encryption];
				if (enc_info) {
					dat.ApCliAuthMode = enc_info[0];
					dat.ApCliEncrypType = enc_info[1];
					
					// APCLI PMF
					if (enc_info[0] == "OWE" || enc_info[0] == "WPA3PSK") {
						dat.ApCliPMFMFPC = "1";
						dat.ApCliPMFMFPR = "1";
						dat.ApCliPMFSHA256 = "0";
					} else {
						dat.ApCliPMFMFPC = "0";
						dat.ApCliPMFMFPR = "0";
					}
				}
				break; // only ONE ApCli supported for each device
			}
		}
	}

	// ------------------------------------------
	// vif => AP setting, set defaults first
	// ------------------------------------------

	// in DAT config, driver expect setting patterns like "1;1;0" for cfgs except suffix-key cfgs
	// we set default strings for each Bssid
	
	// set default vif cfgs
	for (let k, v in defs.VIF_CFGS) {
		let default_str = "";
		for (let i = 0; i < bssid_count; i++) {
			default_str = set_indexed_value(default_str, i, to_str(v));
		}
		dat[k] = default_str;
	}

	// set default ACL cfgs
	for (let k, v in defs.VIF_ACL) {
		for (let i = 0; i < defs.MAX_MBSSID; i++) {
			dat[`${k}${i}`] = to_str(v);
		}
	}

	// clear suffix-key cfgs
	// like SSIDx, WPAPSKx
	// refer to schema/mtwifi/dat-defs.json
	for (let k, v in defs.VIF_CFGS_IDX) {
		for (let i = 1; i <= defs.MAX_MBSSID; i++) {
			dat[`${k}${i}`] = "";
		}
	}

	// ------------------------------------------
	// vif => AP setting, set AP setting for every vif
	// ------------------------------------------

	for (let k, iface in ifaces) {
		let c = ifaces[k].config;
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
				dat[`${key}${suffix_key_idx}`] = to_str(val);
			}
		};

		// common Token cfg setting helper function
		// only set when UCI cfg exists, keep DAT default in other cases
		// sets dat[key] value of current k (also ap_idx)
		// e.g. 12;17;26, sets 17 when k = 1, also now ap_idx = 1
		let set_token = function(key, val) {
			if (val != null) {
				dat[key] = set_indexed_value(dat[key], vif_idx, to_str(val));
			}
		};

		// set suffix-key settings
		set_suffix("SSID", c.ssid);
		set_suffix("WPAPSK", c.key);

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

		// 802.11k/v/w
		set_token("RRMEnable", strict_bool(c.ieee80211k));
		
		// HT settings
		set_token("HT_AMSDU", strict_bool(c.amsdu));
		set_token("HT_AutoBA", strict_bool(c.autoba));
		// for BAWinSize: AX Mode: 256, Non-AX Mode: 64
		set_token("HT_BAWinSize", is_ax ? "256" : "64");
		
		// MU-MIMO / OFDMA
		set_token("MuMimoDlEnable", strict_bool(c.mumimo_dl));
		set_token("MuMimoUlEnable", strict_bool(c.mumimo_ul));
		set_token("MuOfdmaDlEnable", strict_bool(c.ofdma_dl));
		set_token("MuOfdmaUlEnable", strict_bool(c.ofdma_ul));

		// AuthMode + EncrypType
		let enc_def = defs.ENC_2_DAT[c.encryption] || ["OPEN", "NONE"];
		let authmode = enc_def[0];
		set_token("AuthMode", authmode);
		set_token("EncrypType", enc_def[1]);

		// AP PMF
		if (authmode == "OWE" || authmode == "WPA3PSK") {
			set_token("PMFMFPC", "1");
			set_token("PMFMFPR", "1");
			set_token("PMFSHA256", "0");
		} else if (authmode == "WPA2PSKWPA3PSK") {
			set_token("PMFMFPC", "1");
			set_token("PMFMFPR", "0");
			set_token("PMFSHA256", "0");
		} else {
			// NOTE:
			// in VIF_CFGS defaults, they were set to 0
			// override to 0 if there are special cases
		}

        // RekeyMethod
		if (authmode != "OPEN" && authmode != "OWE") {
			set_token("RekeyMethod", "TIME");
		}

		// ACL (MAC Filter)
		// ap_idx -> AccessPolicyx
		// e.g. AccessPolicy0, AccessPolicy1...
		if (c.macfilter == "allow") {
			dat[`AccessPolicy${vif_idx}`] = "1";
		} else if (c.macfilter == "deny") {
			dat[`AccessPolicy${vif_idx}`] = "2";
		} else {
			dat[`AccessPolicy${vif_idx}`] = "0";
		}

		if (c.maclist && length(c.maclist) > 0) {
			// MAC addrs should be like "MAC1;MAC2;MAC3"
			// TODO: maybe we should handle distinct logic here?
			dat[`AccessControlList${vif_idx}`] = join(";", c.maclist);
		}
	}

	return dat;
};