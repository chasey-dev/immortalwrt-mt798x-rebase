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
import * as uci from 'uci';
import * as l1parser from 'l1parser';

import { schemas } from 'mtwifi.defaults';
import * as netifd from 'mtwifi.netifd';
import * as cfg from 'mtwifi.config';
import { log, with_lock } from 'mtwifi.utils';

const LOCK_FILE = "/var/lock/mtwifi.lock";

log.debug(`[Setup] received cmd ${ARGV}`);

let driver_name = ARGV[0];
let command = ARGV[1];
let cur_devname = ARGV[2];
let config_json_str = ARGV[3];

// for netifd script parsing
global.radio = cur_devname;

const types = {
	"array": 1,
	"string": 3,
	"integer": 5,
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
	};

	for (let k, v in schemas) {
		dump[k] = [];
		for (let option in v){
			push(dump[k], dump_option(v, option));
        }
	};

	printf('%J\n', dump);

    exit(0);
}

// ==========================================
//              SETUP
// ==========================================
function handle_setup(data) {
    // we dont need to setup when device is disabled
    if (data.config.disabled) {
        // disable netifd retry 
        netifd.set_retry(false);
        return;
    }

    let l1 = l1parser.open();
    
    // get all devices from L1 Profile
    let all_devs = l1.getall();
    let cur_dev = all_devs[cur_devname];

    if (!cur_dev) {
        netifd.setup_failed("DEVICE_NOT_FOUND");
        l1.close();
        return;
    }
    // inject cur_devname into UCI cfg data
    // UCI doesnt contain this key
    data.device = cur_devname;
    
    /*****        ADD DISABLED VIFS CONFIG       *******/

    // read UCI cfg
    let cursor = uci.cursor();
    cursor.load("wireless");

    // build netifd ifaces projection
    // ifname -> object
    let netifd_ifaces = {};
    for (let k, v in data.interfaces) {
        if (v.name) netifd_ifaces[v.name] = v;
    }

    // rebuild ifaces object from read UCI cfg
    let complete_ifaces = {};
    let sort_idx = 1;

    cursor.foreach("wireless", "wifi-iface", function(sec) {
        // skip iface that doesnt belong to cur dev
        if (sec.device != cur_devname) return;

        // generate ordered keys (01, 02, 03...)
        let key = sprintf("%02d", sort_idx++);

        if (exists(netifd_ifaces, sec['.name'])) {
            // use netifd config if exists
            complete_ifaces[key] = netifd_ifaces[sec['.name']];
        } else {
            // construct iface data with same format
            complete_ifaces[key] = {
                "name": sec['.name'],
                "config": {
                    "network":      split(sec.network, " "),
                    "device":       sec.device,
                    "mode":         sec.mode,
                    "encryption":   sec.encryption,
                    "key":          sec.key,
                    "ssid":         sec.ssid,
                    "radios":       []
                }
            };

            // if current UCI section is disabled, inject config.disabled also
            // here if sec.disabled is null means that it is enabled in config
            complete_ifaces[key].config.disabled = sec.disabled;

            log.debug(`[Setup] Restored disabled interface from UCI: ${sec['.name']}`);
        }
    });

    // replace the data.interfaces
    data.interfaces = complete_ifaces;

    /*****      PREPARE PREFIXES AND COUNTINGS     *******/

    // MTWIFI_AP_IF_PREFIX <= ext_ifname
    // MTWIFI_APCLI_IF_PREFIX <= apcli_ifname
    let ap_prefix = cur_dev.ext_ifname || "ra";         // default to ra
    let apcli_prefix = cur_dev.apcli_ifname || "apcli"; // default to apcli

    // MTWIFI_MAX_AP_IDX=15
    // MTWIFI_MAX_APCLI_IDX=0
    const MAX_AP_IDX = 15;
    const MAX_APCLI_IDX = 0; // e.g. apcli0 ONLY

    let ap_idx = 0;
    let apcli_idx = 0;


    /*****          SET VIFS IN NETIFD        *******/
    
    // netifd idx may mismatch with UCI idx, should maintain it seperately
    let netifd_idx = (() => {
        let i = 1;
        return {
            increase: () => { return ++i; },
            get: () => { return sprintf("%02d", i); }
        }
    })();

    // keep iterating sequence for config.interfaces
    // we assume that UCI arrays are ordered
    // for_each_interface ap mtwifi_vif_ap_set_data
    for (let idx, iface_data in data.interfaces) {
        let config = iface_data.config;
        let mode = config.mode;
        let calc_ifname = null;

        // AP mode handling
        // mtwifi_vif_ap_set_data
        if (mode == "ap") {
            if (ap_idx <= MAX_AP_IDX) {
                calc_ifname = ap_prefix + ap_idx;
                ap_idx++;
            } else {
                log.warn(`[Setup] Ignored AP interface ${idx}: Max index reached.`);
            }
        } 
        // STA(Client) mode handling
        // mtwifi_vif_sta_set_data
        else if (mode == "sta") {
            if (apcli_idx <= MAX_APCLI_IDX) {
                calc_ifname = apcli_prefix + apcli_idx;
                apcli_idx++;
            } else {
                log.warn(`[Setup] Ignored STA interface ${idx}: Max index reached.`);
            }
        }

        // inject calculated ifname into mtwifi_ifname
        // this is CRITICAL for cfg.setup(), since vif names are not contained in raw UCI cfgs
        // json_add_string "$MTWIFI_CFG_IFNAME_KEY" "$ifname"
        if (calc_ifname) {
            iface_data.mtwifi_ifname = calc_ifname;

            // notify netifd to bind interfaces
            // hooked in netifd-wireless
            // mtwifi_vif_ap_config -> wireless_add_vif
            // NOTE: shell script checked config.disabled before wireless_add_vif
            if (!config.disabled) {
                // if previous ifaces were disabled, netifd idx may mismatch with UCI index
                log.info(`[Setup] Add interface: ${idx} -> ${calc_ifname} (mode: ${mode}, netifd idx: ${netifd_idx.get()})`);
                // here set vif with real netifd idx
                netifd.set_vif(netifd_idx.get(), calc_ifname);
                // increase the netifd idx
                netifd_idx.increase();
            } else {
                log.info(`[Setup] Skipped disabled interface: ${calc_ifname}`);
            }
        }
    }

    /*****          SETUP VIFS        *******/
    // UCI => DAT, ifup, reload driver...
	let is_inited = cfg.setup(data, all_devs);
    // notify netifd to setup
	netifd.set_up();

	l1.close();

    // init main vifs may take ~10s, the task will hold the lock until its done
    // if latter setup sessions come up so early, netifd may kill them if it waits too long!!!
    if (!is_inited) {
        sleep(10000);
    }
}

// ==========================================
//              TEARDOWN
// ==========================================
function handle_teardown() {
    let l1 = l1parser.open();
    let all_devs = l1.getall();
    // we dont have to unset vifs in netifd
    // since it is triggered by netifd, vif destroy issues may handled by netifd
    // TODO: teardown logic may still buggy when main device in DBDC were shutdown
	cfg.down(cur_devname, all_devs);
    l1.close();
}

switch (command) {
	case "dump":
		dump_options();
		break;
	case "setup":
		let data = json(config_json_str);
		if(cur_devname && data) {
            with_lock(() => {
                handle_setup(data);
            }, LOCK_FILE, `${command} ${cur_devname}`);
		} else {
            log.error(`[Setup] UCI cfg data not valid!!! raw: ${config_json_str}, json parse: ${data}`);
			exit(1);
		}
		break;
	case "teardown":
        with_lock(() => {
            handle_teardown();
        }, LOCK_FILE, `${command} ${cur_devname}`);
		break;
}