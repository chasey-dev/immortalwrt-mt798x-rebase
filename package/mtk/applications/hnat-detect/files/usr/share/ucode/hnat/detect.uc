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
 *  - Only accept endpoints on MTK GMACs:
 *      (1) GMAC root netdev: of_node compatible contains "mediatek,eth-mac"
 *      (2) Switch (DSA-like) user port netdev:
 *          has phys_switch_id + phys_port_name, and has a lower_* chain to a GMAC root
 *  - LAN follows the primary source zone. LAN2 covers another source endpoint group.
 *  - Rx PPD for Ext devices HNAT, only add to bridge when an ext device is detected.
 */

'use strict';

import * as fs from 'fs';
import * as ubus from 'ubus';
import { log } from 'hnat.utils.common';
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

let zones = fw4.map_zones(state);

let nat_zone = fw4.pick_nat_zone(state, INTERFACE);
let src_zones = nat_zone ? fw4.get_forward_src_zones(nat_zone.name) : [];
if (src_zones == null) {
	log.error('cannot read firewall forwarding configuration');
	exit(1);
}
log.debug(`forward src_zones: ${src_zones} -> nat_zone: ${nat_zone?.name || '-'}`);

/* Prefer "lan", otherwise the first forwarding source, for LAN, PPD and Rx PPD. */
let lan_zone_name = index(src_zones, 'lan') >= 0 ? 'lan' : src_zones[0];

function resolve_zone_endpoints(zone) {
	let endpoints = [];
	for (let dev in fw4.get_zone_devices(zone))
		push(endpoints, ...sysnet.resolve_endpoints(dev, roots));
	return uniq(endpoints);
}

/* Prefer switch prefixes or port names, then GMAC roots. */
function get_lan_names(endpoints) {
	let ports = filter(endpoints, dev => sysnet.is_switch_port_on_gmac(dev, roots));
	let gmacs = filter(endpoints, dev => index(roots, dev) >= 0);
	let names = uniq([
		...map(ports, dev => sysnet.get_switch_prefix(dev, roots) || dev),
		...gmacs,
	]);

	/* The driver matches prefixes, so shorter names cover longer candidates. */
	return filter(names, name => {
		for (let prefix in names) {
			if (prefix == name)
				continue;
			if (index(name, prefix) == 0)
				return false;
		}
		return true;
	});
}

let wan_eps = resolve_zone_endpoints(nat_zone);
let lan_eps = resolve_zone_endpoints(zones[lan_zone_name]);
let all_lan_eps = [ ...lan_eps ];
for (let name in src_zones) {
	if (name != lan_zone_name)
		push(all_lan_eps, ...resolve_zone_endpoints(zones[name]));
}
all_lan_eps = uniq(all_lan_eps);

log.debug(`eps: wan = ${wan_eps}, lan = ${all_lan_eps}`);

/* WAN selection:
 * Prefer PHY root > switch port > any GMAC root.
 * If WAN cannot be resolved (e.g. NAT on apcli0/apclix0), do NOT touch WAN
 */
let wan_gmacs = filter(wan_eps, dev => index(roots, dev) >= 0);
let wan_phys = filter(wan_gmacs, sysnet.has_phy);
let wan_ports = filter(wan_eps, dev => sysnet.is_switch_port_on_gmac(dev, roots));
let wan_name = wan_phys[0] || wan_ports[0] || wan_gmacs[0] || null;

/* LAN selection:
 * Prefer a switch prefix (or port name), otherwise a GMAC root in the primary source.
 * WAN may share the LAN prefix. No LAN -> keep GMAC settings and continue Rx PPD cleanup.
 */
let lan_name = get_lan_names(lan_eps)[0] || null;

if (!lan_name)
	log.warn('skip: cannot resolve LAN endpoint safely');

/* LAN2 selection:
 * - Cover source endpoints not matched by LAN, including DSA ports such as sfp.
 * - Prefer switch prefixes or port names, then GMAC roots. Use "/" when unused.
 */
let lan2_name = '/';
if (lan_name) {
	for (let name in get_lan_names(all_lan_eps)) {
		if (index(name, lan_name) == 0)
			continue;
		lan2_name = name;
		break;
	}
}

/* Uncovered endpoints do not prevent applying settings or cleaning up Rx PPD. */
if (lan_name) {
	let uncovered = filter(all_lan_eps, dev =>
		index(dev, lan_name) != 0 && index(dev, lan2_name) != 0);
	if (length(uncovered))
		log.warn(`LAN/LAN2: uncovered endpoints = ${uncovered}`);
}

/* PPD (Ping-Pong Device) selection (must be an exact GMAC root):
 * - Source zone is switch ports -> PPD = switch's GMAC root
 * - Source zone is PHY -> PPD = that PHY's GMAC root
 * - Source zone is PHY + switch ports -> prefer switch GMAC root
 * - Multiple PHY roots -> use the first GMAC endpoint in the source zone
 */

let lan_ports = filter(lan_eps, dev => sysnet.is_switch_port_on_gmac(dev, roots));
let ppd_name = null;
if (length(lan_ports)) {
	ppd_name = sysnet.resolve_gmac_endpoint(lan_ports[0], roots);
} else {
	ppd_name = filter(lan_eps, dev => index(roots, dev) >= 0)[0] || null;
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
let lan_devs = fw4.get_active_zone_devices(zones[lan_zone_name], network.interface);
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
