/*
 * OpenWrt Firewall 4 based HNAT wan/lan/lan2 interface Setup Utility.
 *
 * Copyright (C) 2026  chasey-dev <ellenyoung0912@gmail.com>
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
 *
 * Basic rules:
 *  - Never write Wi-Fi/USB/virtual device names into HNAT.
 *  - Only accept endpoints on GMAC0/GMAC1:
 *      (1) GMAC root netdev: of_node compatible contains "mediatek,eth-mac"
 *      (2) Switch (DSA-like) user port netdev:
 *          has phys_switch_id + phys_port_name, and has a lower_* chain to a GMAC root
 *  - All switch ports are summarized as a single LAN prefix (e.g. "lan") for driver prefix match.
 *  - LAN2 is an independent PHY GMAC-root (has phydev). If none -> "/".
 *  - Rx PPD for Ext devices HNAT, only add to bridge when an ext device is detected.
 */

'use strict';

import * as fs from 'fs';
import { log, merge } from 'hnat.utils.common';
import * as debugfs from 'hnat.utils.debugfs';
import * as sysnet from 'hnat.utils.sysnet';
import * as fw4 from 'hnat.utils.fw4_parser';

if (!debugfs.is_hnat_present())
	exit(0);

/* ---------------- Env guard ---------------- */

const ACTION    = getenv('ACTION');
const INTERFACE = getenv('INTERFACE');

log.debug(`env: ACTION= ${ACTION || ''} INTERFACE= ${INTERFACE || ''}`);

// if (ACTION != 'ifup' && ACTION != 'update' && ACTION != 'ifupdate')
if (ACTION != 'ifup')
	exit(0);

if (!INTERFACE || INTERFACE == 'loopback')
	exit(0);

/* Serialize interface events before reading the current state. */
let lock = fs.open('/var/run/hnat-detect.lock', 'w');
if (!lock || !lock.lock('x')) {
	log.error(`cannot lock hnat-detect: ${fs.error()}`);
	exit(1);
}

/* ---------------- Main selection ---------------- */

let state = fw4.load_state();
if (!state || !state.zones) {
	log.error('skip: /var/run/fw4.state missing or invalid JSON');
	exit(0);
}

let roots = sysnet.get_gmac_roots();
log.debug('gmac_roots=' + sprintf('%J', roots));

if (!length(roots)) {
	log.error('skip: no GMAC roots (mediatek,eth-mac) found');
	exit(0);
}

let z = fw4.zmap(state);

let nat_zone = fw4.pick_nat_zone(state, INTERFACE);
if (!nat_zone) {
	log.warn('skip: no NAT (masq) zone found');
	exit(0);
}

let src_zones = fw4.forward_src_zones(nat_zone.name);
log.debug(`forward src_zones: ${src_zones} -> nat_zone: ${nat_zone.name}`);

/* pick lan/lan2 zones */
let lan_zone_name = fw4.pick_best(src_zones, [ 'lan' ]);
let lan2_zone_name = null;

if (length(src_zones) > 1) {
	let other = filter(src_zones, s => s != lan_zone_name);
	lan2_zone_name = fw4.pick_best(other, [ 'lan2', 'dmz', 'guest' ]);
}

log.debug(`lan_zone = ${lan_zone_name || '-'}, lan2_zone = ${lan2_zone_name || '-'}`);

/* resolve zone endpoints */
const zone_devs = (zone_name) => {
	let zone = z[zone_name];
	let devs = fw4.zone_devices(zone);

	if (zone && !length(zone.related_physdevs || []) && length(zone.match_devices || []))
		log.debug(`zone ${zone_name}: related_physdevs empty, fallback match_devices = ${devs}`);

	return devs;
};

const zone_eps = (devs) => uniq(merge(...map(devs, d => sysnet.resolve_endpoints(d, roots, 0))));

let nat_zone_devs  = zone_devs(nat_zone.name);
let lan_zone_devs  = lan_zone_name  ? zone_devs(lan_zone_name)  : [];
let lan2_zone_devs = lan2_zone_name ? zone_devs(lan2_zone_name) : [];

let wan_eps  = zone_eps(nat_zone_devs);
let lan_eps  = zone_eps(lan_zone_devs);
let lan2_eps = zone_eps(lan2_zone_devs);

log.debug(`eps: wan = ${wan_eps}, lan = ${lan_eps}, lan2 = ${lan2_eps}`);

/* classify */
const is_phy_root = (d) => (index(roots, d) >= 0) && sysnet.has_phy(d);
const is_sw_port  = (d) => sysnet.is_switch_port_on_gmac(d, roots);
const is_gmac     = (d) => (index(roots, d) >= 0);

let all_sw = filter(lan_eps, (d) => sysnet.is_switch_port_on_gmac(d, roots));
let has_sw = length(all_sw) > 0;

/* WAN selection:
 * Prefer PHY root > switch port > any GMAC root.
 * If WAN cannot be resolved (e.g. NAT on apcli0/apclix0), do NOT touch WAN
 */
let wan_name =
	filter(wan_eps, d => is_phy_root(d))[0] ||
	filter(wan_eps, d => is_sw_port(d))[0]  ||
	filter(wan_eps, d => is_gmac(d))[0]     ||
	null;

/* LAN selection:
 * Prefer switch prefix "lan" (safe) when switch ports exist, else pick GMAC root endpoint from LAN zone.
 */
let lan_name = null;

if (has_sw) {
	let prefix = sysnet.get_switch_prefix(all_sw);

	/* avoid prefix swallowing WAN if WAN itself shares that prefix (rare, but possible) */
	if (prefix && (!wan_name || index(wan_name, prefix) != 0))
		lan_name = prefix;
	else {
		/* fallback: pick one LAN endpoint not equal to WAN */
		lan_name =
			filter(lan_eps, d => is_sw_port(d) && d != wan_name)[0] ||
			filter(lan_eps, d => is_gmac(d) && d != wan_name)[0]    ||
			filter(lan_eps, d => d != wan_name)[0]                  ||
			null;
	}
} else {
	lan_name =
		filter(lan_eps, d => is_gmac(d))[0] ||
		null;
}

if (!lan_name) {
	log.warn('skip: cannot resolve LAN endpoint safely');
	exit(0);
}

/* LAN2 selection:
 * - Exception 1: No switch, 2 PHY roots, WAN is on one of them -> LAN2 must be "/".
 * - Exception 2: No switch, 2 PHY roots, but WAN is NOT on GMAC (e.g. Wi-Fi) -> Allow LAN2.
 */
let lan2_name = '/';

/* Check basic topology for Exception 1 */
let two_phy_roots = (length(roots) == 2 && length(filter(roots, r => sysnet.has_phy(r))) == 2);

/* Exception 2: No switch AND 2 PHY roots AND WAN is actually using a GMAC */
let block_lan2 = (!has_sw && two_phy_roots && wan_name != null);

if (!block_lan2 && length(roots) >= 2) {
	let prefer =
		filter(lan2_eps, d => is_phy_root(d) && d != wan_name && d != lan_name)[0] ||
		filter(roots,    r => sysnet.has_phy(r) && r != wan_name && r != lan_name)[0]  ||
		null;

	if (prefer)
		lan2_name = prefer;
}

/* PPD (Ping-Pong Device) selection (must be an exact GMAC root):
 * - Source zone is switch ports -> PPD = switch's GMAC root
 * - Source zone is PHY -> PPD = that PHY's GMAC root
 * - Source zone is PHY + switch ports -> prefer switch GMAC root
 * - Source zone is PHY + PHY -> keep existing PPD (apcli scenario)
 */

let ppd_name = null;

if (has_sw) {
	ppd_name = sysnet.resolve_gmac_endpoint(all_sw[0], roots);
} else {
	ppd_name = filter(lan_eps, d => is_gmac(d))[0] || null;
}

log.info(`chosen: ppd = ${ppd_name || '(keep)'}, wan = ${wan_name || '(keep)'}, lan = ${lan_name}, lan2 = ${lan2_name}`);

/* Rx PPD detect logic:
 * Rx PPD is used for Ext devices (such as USB, WWAN) HNAT
 * If NAT zone physical device is ext device, add Rx PPD to bridge device in src zone.
 * Else delete Rx PPD from bridge device in src zone.
 */

const RX_PPD_NAME = "rxppd";
const is_ext = (name) => {
    return match(name, /^(usb|wwan|eth)/); 
};
let ext_devs = filter(nat_zone_devs, d => is_ext(d) && !is_gmac(d));

/* cmd queue to exec */
let rx_ppd_cmd = [];

/* find bridge device in lan zone */
let br_dev = null;
let lan_devs = lan_zone_devs;
for (let dev in lan_devs) {
	if (sysnet.is_bridge(dev)) {
		br_dev = dev;
		break;
	}
}

if (length(ext_devs) > 0) {
	if (br_dev) {
		log.info(`ext devices: ${ext_devs}, enable ${RX_PPD_NAME} on ${br_dev}`);
		/* add Rx PPD */
		if (!sysnet.dev_exist(RX_PPD_NAME)) {
			push(rx_ppd_cmd, [ 'ip', 'link', 'add', RX_PPD_NAME, 'type', 'dummy' ]);
			push(rx_ppd_cmd, [ 'ip', 'link', 'set', RX_PPD_NAME, 'up' ]);
		}
		/* add Rx PPD to bridge */
		if (index(sysnet.br_members(br_dev), RX_PPD_NAME) < 0) {
			push(rx_ppd_cmd, [ 'ip', 'link', 'set', RX_PPD_NAME, 'master', br_dev ]);
		}
    }
} else {
	/* no ext devices, remove Rx PPD from bridge */
	if (sysnet.dev_exist(RX_PPD_NAME) && index(sysnet.br_members(br_dev), RX_PPD_NAME) >= 0) {
		push(rx_ppd_cmd, [ 'ip', 'link', 'set', RX_PPD_NAME, 'nomaster' ]);
		log.info(`No ext devices, removing ${RX_PPD_NAME}`);
	}
}

function run_command(cmd) {
	let rc = system(cmd, 5000);
	if (rc != 0)
		log.error(`command failed (${rc}): ${join(' ', cmd)}`);
	return rc == 0;
}

/* Apply:
 * - Write resolved GMAC attributes and execute queued Rx PPD commands.
 * - Suspend enabled hooks during updates, then restore their original state.
 * - Report failed operations. Applied changes are not rolled back.
 */
let hook_state = debugfs.hook_toggle.read();
if (hook_state != 'enabled' && hook_state != 'disabled') {
	log.error('cannot read HNAT hook state');
	exit(1);
}

let next_state = {
	ppd: ppd_name,
	wan: wan_name,
	lan: lan_name,
	lan2: lan2_name,
};
let changes = [];

for (let key, value in next_state) {
	if (!value)
		continue;
	let current = debugfs[key].read();
	if (!current) {
		log.error(`cannot read HNAT ${key} state`);
		exit(1);
	}
	if (value != current)
		push(changes, { key, value });
}

if (!length(changes) && !length(rx_ppd_cmd))
	exit(0);

let failed = false;
/* Disable enabled hooks to clear stale FOE entries before applying changes. */
if (hook_state == 'enabled') {
	if (!debugfs.hook_toggle.write('0'))
		failed = true;
}

if (!failed) {
	/* Apply GMAC attribute changes. */
	for (let change in changes) {
		if (!debugfs[change.key].write(change.value)) {
			log.error(`cannot apply HNAT ${change.key}`);
			failed = true;
		}
	}

	/* Apply Rx PPD commands in dependency order. */
	for (let cmd in rx_ppd_cmd) {
		if (!run_command(cmd)) {
			failed = true;
			break;
		}
	}
}

/* Restore the initial hook state even when an update failed. */
if (hook_state == 'enabled') {
	if (!debugfs.hook_toggle.write('1'))
		failed = true;
}

exit(failed ? 1 : 0);
