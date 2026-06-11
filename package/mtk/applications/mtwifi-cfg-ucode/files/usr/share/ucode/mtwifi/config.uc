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
import { log } from 'mtwifi.utils';
import * as driver from 'mtwifi.driver';
import * as converter from 'mtwifi.converter';

/**
 * Apply one radio's UCI projection to its band DAT and runtime interfaces.
 *
 * Hostapd/wpa_supplicant own AP/STA attach; this function only opens the
 * driver anchor needed for band DAT read and ApCli exposure. Physical-device
 * keys and whole-card module reload are outside this per-radio path.
 *
 * @param {Object} uci_cfg - netifd wireless payload for current radio.
 * @param {Object} all_devs - L1 device map.
 * @returns {boolean} true when DAT/runtime setup completed.
 */
export function setup(uci_cfg, all_devs) {
	// check prerequisites for driver setup
	if (!driver.is_kmod()) return false;

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
	// UCI config ==> DAT apply plan
	let dat_update = converter.convert(uci_cfg);

	/*****       SETTING VIFS     *******/

	let cur_vif_inited = driver.is_vif_inited(cur_dev.main_ifname);

	let vifs = driver.scan_related_vifs(cur_dev);
	log.debug(`[Main] UP vifs related to ${cur_devname}: ${vifs}`);
	for (let vif in vifs) driver.ifdown(vif);

	// Apply explicit deletions first; keys absent from both lists stay untouched.
	for (let key in dat_update.unset)
		ctx.unset(key);
	ctx.merge(dat_update.values);
	ctx.commit();
	ctx.close();
	system("sync");

	if (!cur_vif_inited)
		driver.init_main_vif(cur_dev.main_ifname);

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
