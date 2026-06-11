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

import { log } from 'mtwifi.utils';
import { defs } from 'mtwifi.defaults';
import * as fs from 'fs';

// ==========================================
// iface Operations
// ==========================================

/**
 * Resolve the cfg80211 phy name from an existing netdev.
 *
 * Hostapd/wpad need the phy owner even when mtwifi creates the actual private
 * interface names.
 *
 * @param {string} ifname - Existing netdev name.
 * @returns {string|null} cfg80211 phy name, or null when no phy is attached.
 */
export function phy_from_ifname(ifname) {
	let phy_path = fs.readlink(`/sys/class/net/${ifname}/phy80211`);

	if (!phy_path)
		return null;

	let parts = split(phy_path, "/");

	return parts[length(parts) - 1];
};

/**
 * Wait until a kernel interface appears under /sys/class/net.
 *
 * This is used after driver or wpad operations that create private mtwifi
 * interfaces asynchronously.
 *
 * @param {string} ifname - Interface name to wait for.
 * @returns {boolean} true when the interface appears before timeout.
 */
export function wait_for_iface(ifname) {
	if (!ifname) return false;
	let max_retries = 300;

	let sys_path = `/sys/class/net/${ifname}`;

	for (let i = 0; i < max_retries; i++) {
		if (fs.access(sys_path)) {
			return true;
		}
		sleep(100);
	}
    log.error(`[Driver] Timeout waiting for ${ifname}`);
	return false;
};

/**
 * Bring an interface up after confirming that it exists.
 *
 * @param {string} ifname - Interface name to bring up.
 */
export function ifup(ifname) {
	if (wait_for_iface(ifname)) {
		system(`ifconfig ${ifname} up`);
		log.notice(`[Driver] ifconfig up ${ifname}`);
	}
};

/**
 * Bring an interface down after confirming that it exists.
 *
 * @param {string} ifname - Interface name to bring down.
 */
export function ifdown(ifname) {
	if (wait_for_iface(ifname)) {
		system(`ifconfig ${ifname} down`);
		log.notice(`[Driver] ifconfig down ${ifname}`);
	}
};

/**
 * Initialize the main VIF once so the driver consumes DAT and exposes ApCli.
 *
 * The caller decides whether initialization is needed; this helper only performs
 * the private-driver UP/DOWN sequence.
 *
 * @param {string} ifname - Main interface name.
 */
export function init_main_vif(ifname) {
	log.notice(`[Driver] Init main vif: ${ifname}`);
	ifup(ifname);
	sleep(1000);
	ifdown(ifname);
	log.notice(`[Driver] Init main vif done: ${ifname}`);
};

/**
 * Read IFF_UP from /sys/class/net/<ifname>/flags.
 *
 * @param {string} ifname - Interface name to inspect.
 * @returns {number} 1 when the interface is UP, otherwise 0.
 */
export function get_vif_status(ifname) {
	let flags = fs.readfile(`/sys/class/net/${ifname}/flags`);
	// return 1:UP, 0:DOWN / not exist
	return (flags & 0x1);
};

/**
 * Check whether a VIF has been initialized with a non-zero MAC address.
 *
 * A missing interface returns true to stop later setup from treating an absent
 * VIF as a still-initializing one.
 *
 * @param {string} ifname - Interface name to inspect.
 * @returns {boolean} true when the VIF should be treated as initialized.
 */
export function is_vif_inited(ifname){
	let vif_path = `/sys/class/net/${ifname}/address`;
	if (!fs.access(vif_path)) {
		// stay true if not exist, to prevent later operations
		log.error(`[Driver] is_vif_inited: ${ifname} not found!!!`);
		return true;
	}

	let mac = trim(fs.readfile(vif_path));
	let is_inited = mac && mac != "00:00:00:00:00:00";

	log.debug(`[Driver] is_vif_inited: ${ifname}, mac: ${mac}, is_inited: ${is_inited}`);
	return is_inited;
};

/**
 * Classify an interface name for one L1 device.
 *
 * Match main_ifname before the indexed AP prefix because a name such as ra0
 * matches both main_ifname=ra0 and ext_ifname=ra.
 *
 * @param {Object} dev - L1 device descriptor.
 * @param {string} ifname - Kernel interface name.
 * @returns {string|null} main, ap, sta, or null when unrelated.
 */
function related_ifname_role(dev, ifname) {
	if (dev.main_ifname && ifname == dev.main_ifname)
		return "main";

	if (dev.ext_ifname && match(ifname, regexp(`^${dev.ext_ifname}[0-9]+$`)))
		return "ap";

	if (dev.apcli_ifname && match(ifname, regexp(`^${dev.apcli_ifname}[0-9]+$`)))
		return "sta";

	return null;
}

/**
 * Scan only active VIFs that belong to current device:
 *
 *   main_ifname + ext_ifnameN + apcli_ifnameN, filtered by IFF_UP.
 *
 * Driver cleanup uses this list. Wpad configuration is cleared per PHY.
 *
 * @param {Object} dev - L1 device descriptor:
 *   main_ifname - Main vif name.
 *   ext_ifname - AP vif prefix.
 *   apcli_ifname - ApCli vif prefix.
 * @returns {string[]} Active related ifnames.
 */
export function scan_related_vifs(dev) {
	let sys_ifs = fs.lsdir("/sys/class/net") || [];
	let targets = [];

	for (let ifname in sys_ifs) {
		if (related_ifname_role(dev, ifname) && get_vif_status(ifname))
			push(targets, ifname);
	}

	return targets;
};

// ==========================================
// Driver Operations (modules / iwpriv)
// ==========================================

// ! this is DOWN sequence !
const DRIVERS = ["mtk_warp_proxy", "mtk_warp", "mt_wifi"];

// check if drivers were installed as modules
export function is_kmod() {
	if (!fs.access(`/sys/module/mt_wifi`)) {
		log.error(`[Driver] mt_wifi is buit-in. Install as kmod(s)!!`);
		return false;
	}

	return true;
};

// hard reset driver modules
export function reload() {	
	// uninstall
	for (let drv in DRIVERS) system(`rmmod ${drv}`);
	log.notice("[Driver] Removing Kernel Modules...");

	sleep(2000);

	// install
	for (let drv in reverse(DRIVERS)) system(`modprobe ${drv}`);
	log.notice("[Driver] Installing Kernel Modules...");

	sleep(1000);
};

// iwpriv exec wrapper
export function exec_iwpriv(ifname, key, val) {
	let cmd = `iwpriv ${ifname} set ${key}=${val}`;
	log.debug(`[iwpriv] ${cmd}`);
	system(cmd);
};

// trigger ApCli reconnect
export function trigger_apcli(ifname) {
	exec_iwpriv(ifname, "ApCliEnable", "1");
	exec_iwpriv(ifname, "ApCliAutoConnect", "3");
};

// ==========================================
// Special HW Logics
// ==========================================

// init DBDC main card
// in DBDC cards, init main card first,
// set main iface DOWN and UP
export function init_dbdc_card(ifname) {
	log.notice(`[Driver] Init main vif of DBDC main card: ${ifname}...`);
	ifup(ifname);
	sleep(1000);
	ifdown(ifname);
	log.notice(`[Driver] Init main vif of DBDC main card done!!!`);
};

// apply runtime Hooks
// set iwpriv settings
export function apply_runtime_hooks(iface_cfg, mtwifi_ifname) {
	switch(iface_cfg.mode) {
		case "ap":
			for (let uci_k, v in defs.IWPRIV_AP_CFGS) {
				// v[0]=cmd, v[1]=default
				let val = iface_cfg[uci_k] || v[1];
				exec_iwpriv(mtwifi_ifname, v[0], val);
			}
			break;
		case "sta":
			trigger_apcli(mtwifi_ifname);
			break;
	}
};