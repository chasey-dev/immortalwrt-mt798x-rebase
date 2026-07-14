#!/usr/bin/ucode

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

import * as fs from 'fs';
import * as l1parser from 'l1parser';
import * as datconf from 'datconf';

import { defs, schemas, wpad_overlay } from 'mtwifi.defaults';
import * as netifd from 'mtwifi.netifd';
import * as cfg from 'mtwifi.config';
import * as driver from 'mtwifi.driver';
import { log, with_lock } from 'mtwifi.utils';

import * as hostapd from 'wifi.hostapd';
import * as supplicant from 'wifi.supplicant';
import { validate } from 'wifi.validate';

const LOCK_FILE = "/var/lock/mtwifi.lock";
const MAX_AP_VIFS = defs.MAX_MBSSID;
const MAX_APCLI_VIFS = defs.MAX_APCLI_NUM;
const MAX_MLD_GROUP_ID = defs.MAX_MLD_GROUP_ID;

let command = ARGV[1];
let cur_devname = ARGV[2];
let config_json_str = ARGV[3];

log.debug(`[Setup] received ${command} for ${cur_devname}`);

// for netifd script parsing
global.radio = cur_devname;

const types = {
	"array": 1,
	"string": 3,
	"number": 5,
	"boolean": 7,
};

// ==========================================
//              DUMP
// ==========================================

function dump_option(schema, key) {
	// handle alias types
	let _key = (schema[key].type == 'alias') ? schema[key].default : key;

	// safety check: in case schema types were defined but not found in types const enum
	let type_code = types[schema[_key].type];
	if (!type_code) {
		// fallback to 3
		// TODO: maybe log with warnings?
		type_code = 3;
	}

	return [
		key,
		type_code
	];
}

function dump_options() {
	let dump = {
		"name": "mtwifi", // driver name
		"mlo": {
			"mld_setup": "driver",
			"vif_limit": {
				"ap": MAX_AP_VIFS
			}
		}
	};

	for (let k, v in schemas) {
		dump[k] = [];
		for (let option in v)
			push(dump[k], dump_option(v, option));
	};

	printf('%J\n', dump);

	exit(0);
}

/**
 * Shallow-clone a netifd config object while copying array values.
 *
 * wifi-scripts mutates config during validation and generation, so use a
 * separate copy for wpad input.
 *
 * @param {Object} config - Source config object.
 * @returns {Object} Cloned config object.
 */
function clone_config(config) {
    let res = {};

    for (let k, v in config)
        res[k] = (type(v) == "array") ? [ ...v ] : v;

    return res;
}

/**
 * Clone one interface object for hostapd/wpa_supplicant generation.
 *
 * @param {Object} iface - Source interface object.
 * @returns {Object} Cloned interface with a cloned config object.
 */
function clone_interface(iface) {
    return {
        ...iface,
        config: clone_config(iface.config || {})
    };
}

/**
 * Check whether hostapd and wpa_supplicant are available.
 *
 * @returns {boolean|null} true when hostapd and wpa_supplicant are both
 * available.
 */
function wpad_enabled() {
    return fs.access('/etc/init.d/wpad', 'x') &&
        fs.access('/usr/sbin/hostapd', 'x') &&
        fs.access('/usr/sbin/wpa_supplicant', 'x');
}

/**
 * Normalize device config for wifi-scripts validation.
 *
 * Strip or normalize mtwifi/private values that are valid for UCI/DAT but not
 * for the cfg80211-style validator.
 *
 * @param {Object} config - wifi-device config passed to wifi-scripts.
 */
function normalize_device_config(config) {
    if (config.hwmode && !(config.hwmode in [ "11a", "11b", "11g", "11ad" ]))
        delete config.hwmode;
    if (config.hw_mode && !(config.hw_mode in [ "11a", "11b", "11g", "11ad" ]))
        delete config.hw_mode;

    if (config.htmode == "EHT320-2")
        config.htmode = "EHT320";

    if (config.channel == "auto")
        config.channel = 0;
    else if (config.channel != null)
        config.channel = +config.channel;

    validate("device", config);
}

/**
 * Build the iface shape expected by wifi-scripts.
 *
 * mtwifi_ifname is the real private interface selected earlier from the SDK/L1
 * prefix rules.
 *
 * @param {Object} config - wifi-iface config passed to wifi-scripts.
 * @param {string} ifname - Real mtwifi interface name.
 */
function normalize_iface_config(config, ifname) {
    config.ifname = ifname;

    validate("iface", config);
}

/**
 * Prepare wifi-scripts input for hostapd/wpa_supplicant generation.
 *
 * Overlay keys affect only wpad config output. They are not written back to UCI
 * or DAT.
 *
 * wpad_overlay shape:
 * - device.config: hostapd-wide options.
 * - iface.config: per-BSS defaults applied before UCI/netifd values.
 *
 * @param {Object} data - netifd wireless device payload.
 * @param {Object[]} iface_items - Interfaces to include.
 * @param {string} phy - cfg80211 phy name used by wifi-scripts.
 * @returns {Object} Payload accepted by wifi-scripts.
 */
function prepare_wpad_data(data, iface_items, phy) {
    let wdata = {
        ...data,
        config: clone_config(data.config || {}),
        interfaces: {}
    };
    let device_overlay = wpad_overlay.device;
    let iface_overlay = wpad_overlay.iface;

    /*
     * Device overlay is wpad-only. Scalar keys override this clone.
     * Raw hostapd_options are additive so UCI/netifd options stay visible.
     */
    wdata.config = {
        ...wdata.config,
        ...(device_overlay?.config || {}),
        hostapd_options: [
            ...(wdata.config.hostapd_options || []),
            ...(device_overlay?.config?.hostapd_options || [])
        ]
    };

    wdata.phy = phy;
    let radio = wdata.config.radio;
    radio = (radio == null) ? null : +radio;
    wdata.phy_suffix = (radio != null && radio >= 0) ? ":" + radio : "";
    wdata.vif_phy_suffix = wdata.phy_suffix;
    wdata.ifname_prefix = "";

    normalize_device_config(wdata.config);

    for (let item in iface_items) {
        let iface = item.iface;
        let iface_key = item.key;

        wdata.interfaces[iface_key] = clone_interface(iface);

        let iface_config = wdata.interfaces[iface_key].config;
        /*
         * BSS overlay order is intentional: mtwifi defaults, then the
         * original UCI/netifd interface config.
         */
        iface_config = wdata.interfaces[iface_key].config = {
            ...(iface_overlay?.config || {}),
            ...iface_config
        };

        if (iface_config.mlo && iface_config.mode == "ap") {
            /*
             * MTK MLO AP DAT advertises GCMP-256 and SAE-EXT-KEY for SAE.
             * Set the corresponding wifi-scripts options for wpad.
             */
            if (iface_config.encryption == "sae") {
                iface_config.gcmp256 = true;
                iface_config.sae_ext_key = true;
            }

            iface_config.hostapd_bss_options = [
                ...(iface_config.hostapd_bss_options || []),
                "mtk_private_mld=1"
            ];
        }

        if (item.existing_netdev) {
            iface_config.hostapd_bss_options = [
                ...(iface_config.hostapd_bss_options || []),
                "#existing_netdev"
            ];
        }

        delete iface_config.mlo;
        delete iface_config.mld_addr;
        normalize_iface_config(iface_config, iface.mtwifi_ifname);
    }

    return wdata;
}

/**
 * Register generated configs with hostapd/wpa_supplicant.
 *
 * cfg.setup() handles DAT and the driver-created primary/ApCli interfaces.
 * wifi-scripts registers their per-PHY daemon state.
 *
 * @param {Object} data - netifd wireless device payload with mtwifi_ifname values.
 * @param {Object} cur_dev - L1 device descriptor for current radio.
 * @returns {boolean} true when all enabled AP/STA interfaces were registered.
 */
function setup_wpad(data, cur_dev) {
    let phy = driver.phy_from_ifname(cur_dev.main_ifname);

    if (!phy) {
        netifd.setup_failed("PHY_NOT_FOUND");
        return false;
    }

    let ap_items = [];
    let sta_items = [];

    for (let idx, iface in data.interfaces) {
        let ifname = iface.mtwifi_ifname;
        let config = iface.config;

        if (!ifname)
            continue;

        let iface_key = iface.name || ifname;

        if (config.mode == "ap")
            push(ap_items, {
                iface,
                key: iface_key,
                existing_netdev: ifname == cur_dev.main_ifname
            });
        else if (config.mode == "sta")
            push(sta_items, { iface, key: iface_key });
    }

    let hostapd_data;
    if (length(ap_items)) {
        if (ap_items[0].iface.mtwifi_ifname != cur_dev.main_ifname) {
            netifd.setup_failed('AP_FIRST_BSS_NOT_MAIN');
            return false;
        }

        if (!driver.wait_for_iface(cur_dev.main_ifname)) {
            netifd.setup_failed('AP_IFACE_NOT_FOUND');
            return false;
        }

        hostapd_data = prepare_wpad_data(data, ap_items, phy);
    }

    let supplicant_configs = [];
    let supplicant_data;
    for (let item in sta_items) {
        let iface = item.iface;
        let ifname = iface.mtwifi_ifname;

        if (!driver.wait_for_iface(ifname)) {
            netifd.setup_failed('APCLI_IFACE_NOT_FOUND');
            return false;
        }

        let wdata = prepare_wpad_data(data, [ item ], phy);
        let sconf = supplicant.generate(supplicant_configs, wdata,
            wdata.interfaces[item.key]);
        if (type(sconf) != "object") {
            netifd.setup_failed('SUPPLICANT_CONFIG_FAILED');
            return false;
        }

        sconf.existing_netdev = true;
        supplicant_data = wdata;
    }

    if (length(supplicant_configs))
        supplicant.setup(supplicant_configs, supplicant_data);

    if (hostapd_data)
        hostapd.setup(hostapd_data);

    if (length(supplicant_configs))
        supplicant.start(supplicant_data);

    for (let item in ap_items) {
        let ifname = item.iface.mtwifi_ifname;

        if (!driver.wait_for_iface(ifname)) {
            netifd.setup_failed('AP_IFACE_NOT_FOUND');
            return false;
        }

        driver.apply_runtime_hooks(item.iface.config, ifname);
    }

    return true;
}

/**
 * Clear the per-PHY wpad configuration while retaining driver-created netdevs.
 *
 * @param {Object} dev - L1 device descriptor.
 */
function teardown_wpad(dev) {
    let phy = driver.phy_from_ifname(dev.main_ifname);
    if (!phy)
        return;

    let hostapd_request = { phy, radio: -1, config: "" };
    let supplicant_request = { phy, radio: -1, config: [] };

    if (global.ubus.list('hostapd'))
        global.ubus.call('hostapd', 'config_set', hostapd_request);

    if (global.ubus.list('wpa_supplicant'))
        global.ubus.call('wpa_supplicant', 'config_set', supplicant_request);
}

// ==========================================
//              SETUP
// ==========================================
/**
 * Resolve one L1 device's card wrapper to its band DAT path.
 *
 * l1parser exposes the wrapper path on the first band only. Devices with the
 * same INDEX/mainidx share that wrapper, and subidx N maps to BN(N - 1).
 *
 * @param {Object} dev - Current L1 device descriptor.
 * @param {Object} all_devs - L1 device map.
 * @returns {string} Effective DAT profile path.
 */
function resolve_band_profile_path(dev, all_devs) {
    let profile_key = `BN${int(dev.subidx) - 1}_profile_path`;

    for (let devname, sibling in all_devs) {
        if (sibling.INDEX != dev.INDEX ||
            sibling.mainidx != dev.mainidx ||
            !sibling.profile_path)
            continue;

        let wrapper = datconf.open(sibling.profile_path);
        if (!wrapper)
            continue;

        let profile_path = wrapper.get(profile_key);
        wrapper.close();

        if (profile_path)
            return profile_path;
    }

    return dev.profile_path;
}

function handle_setup(data) {
    let l1 = l1parser.open();

    if (data.config.disabled) {
        // Disabled radios still complete setup after removing stale runtime state.
        let all_devs = l1.getall();
        let cur_dev = all_devs[cur_devname];

        if (cur_dev) {
            teardown_wpad(cur_dev);
            cfg.down(cur_devname, all_devs);
        }

        netifd.set_up();
        l1.close();
        return;
    }


    // get all devices from L1 Profile
    let all_devs = l1.getall();
    let cur_dev = all_devs[cur_devname];

    if (!cur_dev) {
        netifd.setup_failed("DEVICE_NOT_FOUND");
        l1.close();
        return;
    }

    cur_dev.profile_path = resolve_band_profile_path(cur_dev, all_devs);

    // inject cur_devname into UCI cfg data
    // UCI doesnt contain this key
    data.device = cur_devname;

    /*****      PREPARE PREFIXES AND COUNTINGS     *******/

    // MTWIFI_AP_IF_PREFIX <= ext_ifname
    // MTWIFI_APCLI_IF_PREFIX <= apcli_ifname
    let ap_prefix = cur_dev.ext_ifname || "ra";         // default to ra
    let apcli_prefix = cur_dev.apcli_ifname || "apcli"; // default to apcli

    let ap_idx = 0;
    let apcli_idx = 0;


    /*****       Validate and assign vifs      *******/

    // Only accepted interfaces are written to DAT or passed to wpad.
    let active_interfaces = {};

    /*
     * wifi-scripts validates cross-radio mtwifi MLO membership before this
     * per-radio setup. Enforce the vif limits on the current payload.
     */
    for (let idx, iface_data in data.interfaces) {
        let config = iface_data.config;
        let mode = config.mode;

        if (mode == "ap") {
            config.dtim_period ??= int(defs.AP_CFGS.DtimPeriod);

            if (config.mlo) {
                let mld_ifname_match = match(config.ifname, /^ap-mld([0-9]+)$/);
                // ap-mldN uses a zero-based netifd index. DAT group 0 means non-MLO.
                let group_id = mld_ifname_match ? int(mld_ifname_match[1]) + 1 : null;

                if (group_id == null || group_id > MAX_MLD_GROUP_ID) {
                    log.error(`[Setup] Invalid MLO ifname ${config.ifname} ` +
                        `for ${iface_data.name}`);
                    netifd.setup_failed("INVALID_MLO_IFNAME");
                    l1.close();
                    return;
                }
            }

            if (ap_idx >= MAX_AP_VIFS) {
                log.warn(`[Setup] Drop AP interface ${iface_data.name}: ` +
                    `max AP vif count ${MAX_AP_VIFS} reached`);
                continue;
            }

            iface_data.mtwifi_ifname = ap_prefix + ap_idx++;
        }
        else if (mode == "sta") {
            if (apcli_idx >= MAX_APCLI_VIFS) {
                log.warn(`[Setup] Drop STA interface ${iface_data.name}: ` +
                    `max ApCli vif count ${MAX_APCLI_VIFS} reached`);
                continue;
            }

            iface_data.mtwifi_ifname = apcli_prefix + apcli_idx++;
        }

        active_interfaces[idx] = iface_data;
    }

    data.interfaces = active_interfaces;

    /*****          Set vifs in netifd        *******/

    for (let idx, iface_data in data.interfaces) {
        let ifname = iface_data.mtwifi_ifname;
        if (!ifname)
            continue;

        log.info(`[Setup] Add interface: ${idx} -> ${ifname} (mode: ${iface_data.config.mode})`);
        netifd.set_vif(idx, ifname);
    }

    /*****          Set up vifs        *******/
    // Configure DAT and wpad for the active interfaces.
    if (!wpad_enabled()) {
        netifd.setup_failed("WPAD_NOT_FOUND");
        l1.close();
        return;
    }

    teardown_wpad(cur_dev);

    if (!cfg.setup(data, all_devs)) {
        netifd.setup_failed("DAT_SETUP_FAILED");
        l1.close();
        return;
    }

    if (!setup_wpad(data, cur_dev)) {
        teardown_wpad(cur_dev);
        driver.ifdown(cur_dev.main_ifname);
        l1.close();
        return;
    }

    // notify netifd to setup
    netifd.set_up();

    l1.close();
}

// ==========================================
//              TEARDOWN
// ==========================================
function handle_teardown() {
    let l1 = l1parser.open();
    let all_devs = l1.getall();
    let cur_dev = cur_devname ? all_devs[cur_devname] : null;

    if (cur_dev)
        teardown_wpad(cur_dev);

    // netifd removes vifs. This path only clears driver and wpad state.
    // TODO: teardown logic may still be buggy when primary band is shutdown
    cfg.down(cur_devname, all_devs);
    l1.close();
}

switch (command) {
	case "dump":
		dump_options();
		break;
	case "setup":
		let data = json(config_json_str);
		if (cur_devname && data) {
            with_lock(() => {
                handle_setup(data);
            }, LOCK_FILE, `${command} ${cur_devname}`);
		} else {
			log.error(`[Setup] Invalid configuration data for ${cur_devname}`);
			exit(1);
		}
		break;
	case "teardown":
        with_lock(() => {
            handle_teardown();
        }, LOCK_FILE, `${command} ${cur_devname}`);
		break;
}
