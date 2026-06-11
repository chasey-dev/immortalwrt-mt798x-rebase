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

import * as datconf from 'datconf';
import { defs } from 'mtwifi.defaults';
import { log } from 'mtwifi.utils';
import * as driver from 'mtwifi.driver';
import * as converter from 'mtwifi.converter';

/**
 * Find sibling band devices that share the same chip/main index prefix.
 *
 * Example: MT7992_1_1 matches MT7992_1_2, but not MT7992_2_1.
 *
 * @param {string} my_devname - Current L1 device name.
 * @param {string[]} all_devnames - All L1 device names.
 * @returns {string[]} Sibling L1 device names.
 */
function get_sibling_devs(my_devname, all_devnames) {
    let sib_devnames = [];

    // extract prefix
    // regex logic: extract pattern like: ChipName_MainIdx
    // e.g. "MT7981_1_1" => "MT7981_1"
	// parts = [raw_str, prefix]
    let parts = match(my_devname, /^(.*)_\d+$/);

    if (!parts) return [];
    let prefix = parts[1];

    // regex expressions to match sibling dev names
    // starts with prefix e.g. "MT7981_1_", following with number
    let sib_regex = regexp("^" + prefix + "_\\d+$");

    for (let idx, devname in all_devnames) {
        if (devname == my_devname) continue;

        if (match(devname, sib_regex)) {
            push(sib_devnames, devname);
        }
    }
    return sib_devnames;
}

function check_prerequisite() {
	return !driver.is_kmod();
}

/**
 * Compare old and new DAT values and classify driver reload requirements.
 *
 * defs.REINSTALL_CFGS is keyed by DAT key. A matching changed key requires
 * module reload; a key marked preinit must reload even before first band init.
 *
 * @param {Object} dat_old - Existing DAT profile values.
 * @param {Object} dat_new - Converted DAT profile values.
 * @returns {Object} Diff flags: is_changed, need_driver_reload, preinit_reload.
 */
function dat_diff(dat_old, dat_new) {
	let res = {
		"is_changed" : false,
		"need_driver_reload": false,
		"preinit_reload": false
	};

	for (let k, v in dat_new) {
		// k: DAT config key
		// v: new DAT config of current key
		if (v != dat_old[k]) {
			res.is_changed = true;
			// log.debug(`[dat_diff] Key changed: ${k} (${dat_old[k]} -> ${v})`);
			let reload_cfg = defs.REINSTALL_CFGS[k];
			if (reload_cfg) {
				res.need_driver_reload = true;
				if (reload_cfg.preinit)
					res.preinit_reload = true;
				log.notice(`[Driver Reload Trigger] Key changed: ${k} (${dat_old[k]} -> ${v})`);
			}
		}
	}
	return res;
}

/**
 * Apply one radio's UCI projection to its DAT profile and runtime interfaces.
 *
 * Hostapd/wpa_supplicant own AP/STA attach; this function only opens the
 * driver anchor needed for DAT read and ApCli exposure.
 *
 * @param {Object} uci_cfg - netifd wireless payload for current radio.
 * @param {Object} all_devs - L1 device map.
 * @returns {boolean} true when DAT/runtime setup completed.
 */
export function setup(uci_cfg, all_devs) {
	// check prerequisites for driver setup
	if (check_prerequisite()) return true;

	// get current dev name
	let cur_devname = uci_cfg.device;
	// get current dev object by dev name
	let cur_dev = all_devs[cur_devname];

	/*****    UCI CFG => DAT CFG   *******/
	if (!cur_dev.profile_path) {
		log.error(`[Main] Profile not found for ${cur_devname}`);
		return false;
	}

	let ctx = datconf.open(cur_dev.profile_path);
	if (!ctx) {
		log.error(`[Main] Unable to open profile path for ${cur_devname}`);
		return false;
	}
	// get old DAT config
	let dat_old = ctx.getall();

	// UCI config ==> new DAT config
	let dat_new = converter.convert(uci_cfg);

	/*****       SETTING VIFS     *******/

	// prepare DAT diff result first
	// netifd may skip UCI cfgs of disabled vif
	// in hanwckf version, they hacked netifd with patches
	// in our version, we read UCI cfg and compare with netifd parameter
	let diff_res = dat_diff(dat_old, dat_new);
	log.debug(`[Main] dat_diff: ${diff_res}`);

	let all_devnames = keys(all_devs);
	let sib_devnames = get_sibling_devs(cur_devname, all_devnames);
	let has_siblings = length(sib_devnames) > 0;

	let cur_vif_inited = driver.is_vif_inited(cur_dev.main_ifname);

	// collect down devs list
	let down_devnames = [ cur_devname ];

	if (has_siblings && diff_res.need_driver_reload) {
		log.debug(`[Main] Sibling devs of ${cur_devname} : ${sib_devnames}`);

		for (let devname in sib_devnames) push(down_devnames, devname);
	}

	for (let idx, devname in down_devnames) {
		if (diff_res.need_driver_reload)
			driver.unregister_hw_nat(all_devs[devname].main_ifname);

		// scan UP vifs related to dev
		let vifs = driver.scan_related_vifs(all_devs[devname]);
		log.debug(`[Main] UP vifs related to ${devname}: ${vifs}`);
		for (let vif in vifs) driver.ifdown(vif);
	}

	// commit converted DAT config
	ctx.merge(dat_new);
	ctx.commit();
	ctx.close();
	system("sync");

	// Reload kernel modules only for keys that cannot be applied by reopening one band.
	if (diff_res.need_driver_reload) {
		// If the current band has not opened yet, non-preinit keys will be consumed by
		// the first driver profile read. Preinit keys are earlier than that.
		if (!cur_vif_inited && !diff_res.preinit_reload) {
			log.notice("[Driver] Skip module reload before first band init.");
		} else {
			if (!driver.reload())
				return false;
		}
	}

	if (!cur_vif_inited)
		driver.init_main_vif(cur_dev.main_ifname);

	if (!has_siblings || !diff_res.need_driver_reload) return true;

	// Module reload resets the whole card, so restore sibling devs in separate wifi up calls.
	for (let idx, devname in down_devnames) {
		if (!(devname == cur_devname)) {
			// use `&` symbol to run in the background,
			// netifd may handle duplicated `wifi up [dev]` calls
			// current lock context will not pass to it
			system(`/sbin/wifi up ${devname} &`);
		}
	}

	return true;
};

/**
 * Bring down mtwifi runtime interfaces for one radio or all radios.
 *
 * @param {string|null} cur_devname - L1 device name; null tears down all.
 * @param {Object} all_devs - L1 device map.
 */
export function down(cur_devname, all_devs) {
	if(cur_devname) {
		let cur_dev = all_devs[cur_devname];
		let vifs = driver.scan_related_vifs(cur_dev);
		for (let vif in vifs) driver.ifdown(vif);
	} else {
		// loop to DOWN all
		for (let devname, dev in all_devs){
			let vifs = driver.scan_related_vifs(dev);
			for (let vif in vifs) driver.ifdown(vif);
		}
	}
};
