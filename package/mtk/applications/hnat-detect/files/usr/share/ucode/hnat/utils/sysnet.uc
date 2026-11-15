/*
 * /sys/class/net Parse Utility for hnat-detect.
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
 */
'use strict';

import * as fs from 'fs';
import { merge } from 'hnat.utils.common';

const SYS_NET_PATH = '/sys/class/net';
function net_path(dev, sub) {
    return sub ? `${SYS_NET_PATH}/${dev}/${sub}` : `${SYS_NET_PATH}/${dev}`;
}

let _devs  = null;

function get_net_devices() {
    if (_devs == null) {
        _devs = fs.lsdir(SYS_NET_PATH);
        if (_devs == null)
            die(`cannot list network devices: ${fs.error()}`);
    }
    return _devs;
}

function is_gmac_root(dev) {
    let c = fs.readfile(net_path(dev, 'of_node/compatible'));
    if (!c)
        c = fs.readfile(net_path(dev, 'device/of_node/compatible'));
    return (c && index(c, 'mediatek,eth-mac') >= 0);
};

export function get_gmac_roots() {
    return filter(get_net_devices(), is_gmac_root);
};

export function dev_exist(dev) {
    return fs.stat(net_path(dev)) != null;
};

export function is_up(dev) {
    return !!(int(fs.readfile(net_path(dev, 'flags')), 16) & 1);
};

export function get_master(dev) {
    let link = fs.readlink(net_path(dev, 'master'));
    return link ? fs.basename(link) : null;
};

function get_lower_devs(dev) {
    let ents = fs.lsdir(net_path(dev)) || [];
    let lowers = filter(ents, n => index(n, 'lower_') == 0);
    return map(lowers, n => substr(n, 6));
};

export function is_bridge(dev) {
    return fs.stat(net_path(dev, 'bridge')) != null;
};

export function br_members(br) {
    return fs.lsdir(net_path(br, 'brif')) || [];
};

export function has_phy(dev) {
    return (fs.readlink(net_path(dev, 'phydev')) != null);
};

/*
 * Switch (DSA-like) user port detection:
 * - do NOT rely on /dsa/*, because some systems don't expose it
 * - use phys_switch_id + phys_port_name existence
 */
export function is_switch_port(dev) {
    return fs.readfile(net_path(dev, 'phys_switch_id')) != null &&
        fs.readfile(net_path(dev, 'phys_port_name')) != null;
};

function get_switch_master_on_gmac(dev, roots, depth) {
    if (depth == null) depth = 0;
    if (depth > 8) return null;

    let lowers = get_lower_devs(dev);
    for (let l in lowers)
        if (index(roots, l) >= 0)
            return l;

    for (let l in lowers) {
        let r = get_switch_master_on_gmac(l, roots, depth + 1);
        if (r) return r;
    }

    return null;
}

export function is_switch_port_on_gmac(port, roots) {
    return (is_switch_port(port) && get_switch_master_on_gmac(port, roots) != null);
};

/*
 * Resolve a NAT zone device to Ext device names.
 * - Require administrative UP state and exclude MTK GMACs and their switch ports.
 * - Follow lower_* links through upper devices such as VLANs and bridges.
 * - At devices without lowers, accept usb/wwan/eth name prefixes.
 * - Return unique names. Detection uses names, not bus or driver information.
 */
export function resolve_ext_devices(dev, roots, depth) {
    if (depth == null) depth = 0;
    if (depth > 12 || !is_up(dev)) return [];
    if (index(roots, dev) >= 0 || is_switch_port_on_gmac(dev, roots)) return [];

    let lowers = get_lower_devs(dev);
    if (length(lowers))
        return uniq(merge(...map(lowers, d => resolve_ext_devices(d, roots, depth + 1))));

    return match(dev, /^(usb|wwan|eth)/) ? [ dev ] : [];
};

/*
 * Map an upper device to physical devices.
 * - bridge: resolve physical devices respectively
 * - DSA port: return itself
 * - PHY connect to GMAC: return itself
 */
export function resolve_endpoints(dev, roots, depth) {
    if (depth == null) depth = 0;
    if (depth > 12) return [];
    if (!dev || fs.stat(net_path(dev)) == null) return [];

    if (is_bridge(dev)) {
        let mem = br_members(dev);
        return uniq(merge(...map(mem, m => resolve_endpoints(m, roots, depth + 1))));
    }

    if (is_switch_port_on_gmac(dev, roots))
        return [ dev ];

    if (index(roots, dev) >= 0)
        return [ dev ];

    let lowers = get_lower_devs(dev);
    if (!length(lowers))
        return [];

    return uniq(merge(...map(lowers, l => resolve_endpoints(l, roots, depth + 1))));
};

/*
 * Map an allowed endpoint to its owning GMAC root.
 * - GMAC root endpoint: itself
 * - Switch port endpoint: its lower-chain GMAC root
 * - Otherwise: null
 */
export function resolve_gmac_endpoint(dev, roots) {
    if (!dev) return null;
    if (index(roots, dev) >= 0)
        return dev;

    if (is_switch_port(dev))
        return get_switch_master_on_gmac(dev, roots);

    return null;
};

/*
 * Compute a safe numeric prefix for switch ports (e.g. lan1/lan2 -> "lan").
 * Every matching netdev must be a switch port on an MTK GMAC.
 * This avoids accidentally matching Wi-Fi/USB/etc.
 */
export function get_switch_prefix(port, roots) {
    let m = match(port, /^([A-Za-z_-]+)[0-9]+$/);
    if (!m) return null;
    let pref = m[1];

    /* Do not include devices outside the supported GMAC topology. */
    for (let d in get_net_devices()) {
        if (index(d, pref) == 0 && !is_switch_port_on_gmac(d, roots))
            return null;
    }

    return pref;
};
