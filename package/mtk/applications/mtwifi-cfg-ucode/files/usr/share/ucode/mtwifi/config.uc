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

import { task } from 'uloop';

import * as datconf from 'datconf';
import { defs } from 'mtwifi.defaults';
import { log } from 'mtwifi.utils';
import * as driver from 'mtwifi.driver';
import * as converter from 'mtwifi.converter';

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

// sync chip config to sibling devs
function sync_chip_config(src_dat, sib_devnames, all_devs) {
	for (let idx, devname in sib_devnames) {
		let sib_info = all_devs[devname];

		if (!sib_info.profile_path) continue;
		let sib_dat = datconf.open(sib_info.profile_path);
		if (sib_dat) {
			let s_updates = {};
			for (let k, v in defs.CHIP_CFGS) {
				// v[0] for DAT key (e.g., WHNAT, BeaconPeriod)
				let dat_key = v[0];
				if (exists(src_dat, dat_key)) {
					s_updates[dat_key] = src_dat[dat_key];
					// log.debug(`[Sync] key to sync: ${dat_key}, src_dat val: ${src_dat[dat_key]}, s_updates val: ${s_updates[dat_key]}`);
				}
			}
			log.debug(`[Sync] s_updates: keys_raw: ${s_updates}, len: ${length(s_updates)}`);
			if (length(s_updates) > 0) {
				log.info(`[Sync] Syncing chip configs to ${devname}`);
				sib_dat.merge(s_updates);
				sib_dat.commit();
			}
			sib_dat.close();
		}
	}
}

function check_prerequisite() {
	return !driver.is_kmod();
}

function dat_diff(dat_old, dat_new) {
	// prepare hashtable for O(1) lookups
	let res = {
		"is_changed" : false,
		"need_reload": false
	};

	let reload_lookup = {};
	for (let k in defs.REINSTALL_CFGS) reload_lookup[k] = true;

	for (let k, v in dat_new) {
		// k: DAT config key
		// v: new DAT config of current key
		if (v != dat_old[k]) {
			res.is_changed = true;
			// log.debug(`[dat_diff] Key changed: ${k} (${dat_old[k]} -> ${v})`);
			if (reload_lookup[k]) {
				res.need_reload = true;
				log.notice(`[Reload Trigger] Key changed: ${k} (${dat_old[k]} -> ${v})`);
				// we have collected all needed flags
				break;
			}
		}
	}
	return res;
}

export function setup(uci_cfg, all_devs) {
	// check prerequisites for driver setup
	if (check_prerequisite()) return;

	// get current dev name
	let cur_devname = uci_cfg.device;
	// get current dev object by dev name
	let cur_dev = all_devs[cur_devname];

	/*****    UCI CFG => DAT CFG   *******/
	if (!cur_dev.profile_path) {
		log.error(`[Main] Profile not found for ${cur_devname}`);
		return;
	}

	let ctx = datconf.open(cur_dev.profile_path);
	if (!ctx) {
		log.error(`[Main] Unable to open profile path for ${cur_devname}`);
		return;
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

	// collect down devs list
	let down_devnames = [ cur_devname ];
	
	let is_dbdc = (index(cur_dev.profile_path, "dbdc") >= 0);
	// only add sibling devs when DAT has changed
	if (is_dbdc && diff_res.is_changed) {
		// find sibling devs for current dev
		let all_devnames = keys(all_devs);
		let sib_devnames = get_sibling_devs(cur_devname, all_devnames);
		log.debug(`[Main] Sibling devs of ${cur_devname} : ${sib_devnames}`);

		// sync current chip config to other sibling devs
		if (uci_cfg.config.dbdc_main) {
			sync_chip_config(dat_new, sib_devnames, all_devs);
		}
		for (let devname in sib_devnames) push(down_devnames, devname);
	}

	// collect vifs needed to be restored
	// if DAT is changed! => UP vifs of ALL sibling devs,
	let restore_vifs = [];
	for (let idx, devname in down_devnames) {
		// skip current device in netifd context, vifs of which will be handled seperately
		let need_restore = !(devname == cur_devname);
		// scan UP vifs related to dev
		let vifs = driver.scan_related_vifs(all_devs[devname]);
		log.debug(`[Main] UP vifs related to ${devname}: ${vifs}, need restore: ${need_restore}`);
		for (let vif in vifs) {
			driver.ifdown(vif);
			if (need_restore) push(restore_vifs, vif);
		}
	}

	// commit converted DAT config
	ctx.merge(dat_new);
	ctx.commit();
	ctx.close();
	system("sync");

	// reload driver if needed
	if (diff_res.need_reload) {
		driver.reload();
	}
	
	// no matter if it is DBDC card, we assume ifup main_vif is time-costy
	// we dont want latter netifd setup sessions come up so early
	// so we check here and decide whether we should sleep after creating the async task
	let main_vif = "";
	if (is_dbdc) {
		// concat main dev name: ChipName_ChipIndex_1
		let main_devname = `${cur_dev.INDEX}_${cur_dev.mainidx}_1`;
		main_vif = all_devs[main_devname].main_ifname;
	} else {
		main_vif = cur_dev.main_ifname;
	}
	let is_inited = driver.is_vif_inited(main_vif);

	// post setup may need ~20s
	// netifd kills main process if it waits too long, create a async task to do that
	const postTask = task(() => {
		// for DBDC cards, you need to init main dev first
		if (is_dbdc && !is_inited) {
			// just init main vif !
			driver.init_dbdc_card(main_vif);
		}

		// set vifs in current cfg UP first
		// implict trace in raw netifd parameter:
		// DISABLED vifs are not contained in uci_cfg.interfaces
		// uci_cfg.interfaces will be EMPTY when uci_cfg.disabled = true, that current dev is disabled
		// current trace:
		// we read UCI cfg and added disabled ifaces from caller script
		for (let idx, iface in uci_cfg.interfaces) {
			let vif = iface.mtwifi_ifname;
			let vif_cfg = iface.config;
			log.debug(`[UCI] idx:${idx}, vif: ${vif}, disabled: ${vif_cfg.disabled ? true : false}, iface: ${iface}, iface cfg:${vif_cfg}`);

			if (vif && !vif_cfg.disabled) {
				driver.ifup(vif);
				driver.apply_runtime_hooks(vif_cfg, vif);
			}
		}
		
		// for non DBDC cards, all set and return!
		if (!is_dbdc) return;

		// for DBDC cards, restore sibling devs
		if (diff_res.need_reload) {
			// in driver reload situation, 
			// let another process to handle sib devs setup
			for (let idx, devname in down_devnames) {
				if (!(devname == cur_devname)) {
					// use `&` symbol to run in the background,
					// netifd may handle duplicated `wifi up [dev]` calls
					// current lock context will not pass to it
					system(`/sbin/wifi up ${devname} &`);
				}
			}
		} else {
			// in normal situation,
			// for DBDC cards, restore vifs of sibling devs (if they were added before)
			// just restore them in no need for reload situation
			for (let vif in restore_vifs) {
				log.notice(`[Main] Restoring sibling vif: ${vif}`);
				driver.ifup(vif);

				// for apcli vifs, do apcli triggers
				if (index(vif, "apcli") >= 0) {
					driver.trigger_apcli(vif);
				}
			}
		}
	});

	// after task is created, just return and notify netifd
	log.info(`[Main] Recovering vifs, create a task to do so. Task PID: ${postTask.pid()}`);

	// let caller script know that main vif is inited or not
	return is_inited;
};

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
