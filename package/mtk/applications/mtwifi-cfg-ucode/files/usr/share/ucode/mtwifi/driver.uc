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
 * Collect all related ifnames by role, including DOWN interfaces:
 *
 *   main: main_ifname, the anchor interface passed to hostapd config_add
 *   ap:   ext_ifnameN vifs for hostapd
 *   sta:  apcli_ifnameN vifs for wpa_supplicant
 *
 * @param {Object} dev - L1 device descriptor:
 *   main_ifname - Main vif name.
 *   ext_ifname - AP vif prefix.
 *   apcli_ifname - ApCli vif prefix.
 * @returns {Object} Related ifnames by role: main string, ap string[], sta string[].
 */
export function related_ifnames(dev) {
	let sys_ifs = fs.lsdir("/sys/class/net") || [];
	let res = {
		main: dev.main_ifname,
		ap: [],
		sta: []
	};

	for (let ifname in sys_ifs) {
		switch (related_ifname_role(dev, ifname)) {
		case "main":
			res.main = ifname;
			break;
		case "ap":
			push(res.ap, ifname);
			break;
		case "sta":
			push(res.sta, ifname);
			break;
		}
	}

	return res;
};

/**
 * Scan only active VIFs that belong to current device:
 *
 *   main_ifname + ext_ifnameN + apcli_ifnameN, filtered by IFF_UP.
 *
 * This is for driver cleanup; wpad teardown uses related_ifnames() instead.
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

/**
 * Check whether a kernel module is currently loaded.
 *
 * @param {string} name - Kernel module name.
 * @returns {boolean} true when /sys/module/<name> exists.
 */
function module_loaded(name) {
	return fs.access(`/sys/module/${name}`);
}

// Module names in unload order. Do not unload mtk_warp here; split WiFi7
// targets fast-fail while it is loaded.
const WIFI_MODULES = [
	"mt7993", "mt7992", "mt7990",
	"mtk_wed",
	"mtk_pci",
	"connac_if",
	"mtk_hwifi",
	"mt7915_mt_wifi",
	"mt_wifi",
	"mt_wifi_osal", "mt_wifi_cmn"
];

const WIFI7_CHIP_MODULES = [ "mt7993", "mt7992", "mt7990" ];
const COMMON_MODULES = [ "mt_wifi_osal", "mt_wifi_cmn" ];

/**
 * Build the loaded WiFi module list in unload order.
 *
 * The plan is intentionally conservative: it requires mt_wifi, rejects mixed
 * common layers, and refuses split WiFi7 reload while mtk_warp is loaded.
 *
 * @returns {(string[]|null)} Module names in unload order, or null on unsafe state.
 */
function reload_plan() {
	let modules = [];
	let common = null;
	let has_wifi7_chip = false;

	for (let mod in WIFI_MODULES) {
		if (!module_loaded(mod))
			continue;

		if (index(COMMON_MODULES, mod) >= 0) {
			if (common) {
				log.error("[Driver] Both mt_wifi_osal and mt_wifi_cmn are loaded, skip mixed common layer reload");
				return null;
			}
			common = mod;
		}

		if (index(WIFI7_CHIP_MODULES, mod) >= 0)
			has_wifi7_chip = true;

		push(modules, mod);
	}

	if (index(modules, "mt_wifi") < 0) {
		log.error("[Driver] mt_wifi is not loaded, skip partial module reload");
		return null;
	}

	if (has_wifi7_chip && module_loaded("mtk_warp")) {
		log.error("[Driver] Split WiFi7 module reload is unsafe while mtk_warp is loaded");
		return null;
	}

	return modules;
}

/**
 * Read module options from /etc/modules.d for one module.
 *
 * @param {string} name - Kernel module name.
 * @returns {string} Raw option string, or an empty string when none is found.
 */
function module_options(name) {
	let files = fs.glob("/etc/modules.d/*") || [];

	for (let file in files) {
		let data = fs.readfile(file);
		if (!data)
			continue;

		for (let line in split(data, "\n")) {
			line = trim(line);
			if (!line || substr(line, 0, 1) == "#")
				continue;

			let fields = split(line, /\s+/, 2);
			if (fields[0] == name)
				return trim(fields[1] || "");
		}
	}

	return "";
}

/**
 * Build modprobe argv with /etc/modules.d options preserved.
 *
 * @param {string} name - Kernel module name.
 * @returns {string[]} argv passed to system().
 */
function module_load_args(name) {
	let args = [ "modprobe", name ];
	let opts = module_options(name);

	if (opts) {
		for (let opt in split(opts, /\s+/))
			push(args, opt);
	}

	return args;
}

/**
 * Check whether the mtwifi driver stack is available as loaded modules.
 *
 * @returns {boolean} true when mt_wifi is loaded.
 */
export function is_kmod() {
	if (!module_loaded("mt_wifi")) {
		log.error("[Driver] mt_wifi module is not loaded.");
		return false;
	}

	return true;
};

/**
 * Disable HW NAT registration for one existing interface.
 *
 * Missing or empty ifnames are ignored because teardown paths may call this
 * after an interface is already gone.
 *
 * @param {string} ifname - Interface name.
 */
export function unregister_hw_nat(ifname) {
	if (!ifname || !fs.access(`/sys/class/net/${ifname}`))
		return;

	exec_mwctl(ifname, "hw_nat_register", "0");
};

/**
 * Reload the live mtwifi module stack using the conservative reload plan.
 *
 * Modules are removed in dependency order and loaded back in reverse order.
 * Module options are preserved through module_load_args().
 *
 * @returns {boolean} true when unload and reload both complete.
 */
export function reload() {
	let modules = reload_plan();
	if (!modules)
		return false;

	log.notice(`[Driver] Removing Kernel Modules: ${join(" ", modules)}`);
	for (let mod in modules) {
		let rc = system([ "rmmod", mod ]);
		if (rc != 0) {
			log.error(`[Driver] rmmod ${mod} failed: ${rc}`);
			return false;
		}
	}

	sleep(2000);

	let load_modules = reverse(modules);
	log.notice(`[Driver] Installing Kernel Modules: ${join(" ", load_modules)}`);
	for (let mod in load_modules) {
		let rc = system(module_load_args(mod));
		if (rc != 0) {
			log.error(`[Driver] modprobe ${mod} failed: ${rc}`);
			return false;
		}
	}

	sleep(1000);
	return true;
};

/**
 * Run one mtwifi mwctl set command.
 *
 * @param {string} ifname - Interface name.
 * @param {string} key - iwpriv key.
 * @param {any} val - iwpriv value serialized into key=value.
 */
export function exec_mwctl(ifname, key, val) {
	let cmd = `mwctl ${ifname} set ${key}=${val}`;
	log.debug(`[mwctl] ${cmd}`);
	system(cmd);
};

/**
 * Trigger ApCli reconnect on an existing client interface.
 *
 * @param {string} ifname - ApCli interface name.
 */
export function trigger_apcli(ifname) {
	exec_mwctl(ifname, "ApCliEnable", "1");
	exec_mwctl(ifname, "ApCliAutoConnect", "3");
};

/**
 * Apply AP-only runtime iwpriv hooks after hostapd brings the VIF up.
 *
 * These hooks cover private driver controls that are still outside hostapd DAT
 * generation. Non-AP interfaces are ignored.
 *
 * @param {Object} iface_cfg - wifi-iface config projected from UCI:
 *   mode - UCI interface mode, ap or sta.
 * @param {string} mtwifi_ifname - Real mtwifi interface name.
 */
export function apply_runtime_hooks(iface_cfg, mtwifi_ifname) {
	if (iface_cfg.mode != "ap")
		return;

	for (let uci_k, v in defs.IWPRIV_AP_CFGS) {
		// v[0]=cmd, v[1]=default
		let val = iface_cfg[uci_k] || v[1];
		exec_mwctl(mtwifi_ifname, v[0], val);
	}
};
