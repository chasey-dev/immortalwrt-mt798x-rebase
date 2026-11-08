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
import * as ubus from 'ubus';
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

if (index([ 'ifup', 'ifdown', 'ifupdate', 'update', 'reload' ], ACTION) < 0)
	exit(0);

if (ACTION != 'reload' && (!INTERFACE || INTERFACE == 'loopback'))
	exit(0);

/* Serialize hotplug and explicit reloads before reading the current state. */
let lock = fs.open('/var/run/hnat-detect.lock', 'w');
if (!lock || !lock.lock('x')) {
	log.error(`cannot lock hnat-detect: ${fs.error()}`);
	exit(1);
}

/* ---------------- Main selection ---------------- */

let state = fw4.load_state();
if (type(state?.zones) != 'array') {
	log.error('skip: /var/run/fw4.state missing or invalid JSON');
	exit(1);
}

let roots = sysnet.get_gmac_roots();
log.debug('gmac_roots=' + sprintf('%J', roots));

if (!length(roots))
	log.warn('no GMAC roots found, keeping HNAT interface settings');

let conn = ubus.connect(null, 5);
let network = conn?.call('network.interface', 'dump', {});
let network_error = ubus.error(true);
conn?.disconnect();
if (type(network?.interface) != 'array') {
	/* S19firewall runs before netifd. Interface hotplug applies HNAT later. */
	if (ACTION == 'reload' && network == null && network_error == ubus.STATUS_NOT_FOUND) {
		log.info('defer HNAT setup until netifd registers network.interface');
		exit(0);
	}
	log.error(`cannot read netifd interface state: ubus status ${network_error}`);
	exit(1);
}

let z = fw4.zmap(state);

let nat_zone = fw4.pick_nat_zone(state, INTERFACE);
let src_zones = nat_zone ? fw4.forward_src_zones(nat_zone.name) : [];
if (src_zones == null) {
	log.error('cannot read firewall forwarding configuration');
	exit(1);
}
log.debug(`forward src_zones: ${src_zones} -> nat_zone: ${nat_zone?.name || '-'}`);

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

let nat_zone_devs  = zone_devs(nat_zone?.name);
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

if (!lan_name)
	log.warn('skip: cannot resolve LAN endpoint safely');

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

/* Rx PPD configuration:
 * - Active Ext devices + LAN endpoint -> first active bridge in the source zone.
 * - No target bridge -> detach Rx PPD.
 */
const RX_PPD_NAME = "rxppd";

/* Resolve active Ext devices in the NAT zone. */
let active_devs = fw4.get_active_zone_devices(nat_zone, network.interface);
let ext_devs = [];
for (let dev in active_devs)
	push(ext_devs, ...sysnet.resolve_ext_devices(dev, roots));
ext_devs = uniq(ext_devs);

/* Select a source bridge when both Ext and LAN endpoints are available. */
let rx_ppd_bridge = null;
let lan_devs = fw4.get_active_zone_devices(z[lan_zone_name], network.interface);
if (length(ext_devs) && lan_name) {
	for (let dev in lan_devs) {
		if (sysnet.is_up(dev) && sysnet.is_bridge(dev)) {
			rx_ppd_bridge = dev;
			break;
		}
	}
}

let rx_ppd_master = sysnet.get_master(RX_PPD_NAME);
let rx_ppd_cmd = [];

/* Detach from the old bridge when the target changes or disappears. */
if (rx_ppd_master && rx_ppd_master != rx_ppd_bridge)
	push(rx_ppd_cmd, [ 'ip', 'link', 'set', RX_PPD_NAME, 'nomaster' ]);

if (rx_ppd_bridge) {
	/* Create the dummy device when needed. */
	if (!sysnet.dev_exist(RX_PPD_NAME))
		push(rx_ppd_cmd, [ 'ip', 'link', 'add', RX_PPD_NAME, 'type', 'dummy' ]);

	/* Attach to the target bridge or restore UP on the current bridge. */
	if (rx_ppd_master != rx_ppd_bridge || !sysnet.is_up(RX_PPD_NAME))
		push(rx_ppd_cmd, [ 'ip', 'link', 'set', RX_PPD_NAME, 'up', 'master', rx_ppd_bridge ]);
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

/* Without a LAN endpoint, keep the current GMAC interface attributes. */
if (lan_name) {
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

	/* Verify the final Rx PPD bridge and link state. */
	if (length(rx_ppd_cmd)) {
		if (sysnet.get_master(RX_PPD_NAME) != rx_ppd_bridge) {
			log.error('Rx PPD bridge did not match the requested configuration');
			failed = true;
		} else if (rx_ppd_bridge && !sysnet.is_up(RX_PPD_NAME)) {
			log.error('Rx PPD is not UP on the target bridge');
			failed = true;
		} else {
			log.info(`ext devices: ${ext_devs}, ${RX_PPD_NAME} bridge: ${rx_ppd_bridge || '(none)'}`);
		}
	}
}

/* Restore the initial hook state even when an update failed. */
if (hook_state == 'enabled') {
	if (!debugfs.hook_toggle.write('1'))
		failed = true;
}

exit(failed ? 1 : 0);
