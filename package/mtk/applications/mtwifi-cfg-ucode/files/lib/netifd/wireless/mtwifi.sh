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

import { defs, schemas } from 'mtwifi.defaults';
import * as netifd from 'mtwifi.netifd';
import * as cfg from 'mtwifi.config';
import * as driver from 'mtwifi.driver';
import { log, with_lock } from 'mtwifi.utils';

const LOCK_FILE = "/var/lock/mtwifi.lock";
const MAX_AP_VIFS = defs.MAX_MBSSID;
const MAX_APCLI_VIFS = defs.MAX_APCLI_NUM;

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
	};

	for (let k, v in schemas) {
		dump[k] = [];
		for (let option in v)
			push(dump[k], dump_option(v, option));
	};

	printf('%J\n', dump);

	exit(0);
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

    // Enforce the vif limits on the current netifd payload.
    for (let idx, iface_data in data.interfaces) {
        let config = iface_data.config;
        let mode = config.mode;

        if (mode == "ap") {
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
    // Configure DAT for the active interfaces.
    if (!cfg.setup(data, all_devs)) {
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
