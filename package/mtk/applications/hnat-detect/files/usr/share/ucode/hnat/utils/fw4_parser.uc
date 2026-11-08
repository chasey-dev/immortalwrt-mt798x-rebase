/*
 * OpenWrt Firewall 4 Parse Utility For hnat-detect.
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
import * as uci from 'uci';

const STATE_PATH = "/var/run/fw4.state";

export function load_state() {
    let s = fs.readfile(STATE_PATH);
    if (!s) return null;
    try { return json(s); } catch (e) { return null; }
};

export function map_zones(state) {
    let zones = {};
    for (let zone in state.zones)
        zones[zone.name] = zone;
    return zones;
};

function device_matches(dev, pattern) {
    if (match(pattern, /[+*]$/))
        return index(dev, substr(pattern, 0, length(pattern) - 1)) == 0;
    return dev == pattern;
}

function get_matching_devices(specs) {
    if (!length(specs))
        return [];

    let result = [];
    let devices = fs.lsdir('/sys/class/net');
    if (devices == null)
        die(`cannot list network devices: ${fs.error()}`);

    /* Collect existing devices matched by positive names or prefixes. */
    for (let spec in specs) {
        if (spec.invert)
            continue;
        let pattern = spec.device || spec.val;
        push(result, ...filter(devices, dev => device_matches(dev, pattern)));
    }

    /* Apply exclusions last, regardless of their position in the zone. */
    for (let spec in specs) {
        if (!spec.invert)
            continue;
        let pattern = spec.device || spec.val;
        result = filter(result, dev => !device_matches(dev, pattern));
    }

    return uniq(result);
}

/*
 * Prefer resolved physical devices for logical networks, then add explicit devices.
 * Expand wildcard matches and apply explicit exclusions to both sources.
 */
export function get_zone_devices(zone) {
    if (!zone)
        return [];

    let phys = zone.related_physdevs || [];
    let names = length(phys) ? phys : zone.match_devices || [];
    let specs = map(names, device => ({ device }));
    return get_matching_devices([ ...specs, ...(zone.device || []) ]);
};

/*
 * Use netifd for active networks because fw4's cached devices may be stale.
 * Keep inverted networks as exclusions even when they are down.
 */
export function get_active_zone_devices(zone, interfaces) {
    if (!zone)
        return [];

    let specs = [];
    for (let net in (zone.network || [])) {
        let ifname = net.device || net.val;
        for (let iface in interfaces) {
            if (iface.interface != ifname)
                continue;
            if (!net.invert && !iface.up)
                continue;

            for (let dev in [iface.device, iface.l3_device]) {
                if (dev)
                    push(specs, { device: dev, invert: net.invert });
            }
        }
    }

    return get_matching_devices([ ...specs, ...(zone.device || []) ]);
};

export function zone_has_iface(zone, ifname) {
    if (!ifname)
        return false;

    let nets = zone.network || [];
    for (let net in nets) {
        if (net.invert)
            continue;
        if (net.val == ifname || net.device == ifname)
            return true;
    }
    return false;
};

export function pick_nat_zone(state, ifname) {
    let masq = filter(state.zones || [], z => z.masq || z.masq6);
    if (!length(masq)) return null;

    /* prefer name "wan" */
    for (let z in masq)
        if (z.name == 'wan')
            return z;

    /* else prefer the one owning current iface */
    for (let z in masq)
        if (zone_has_iface(z, ifname))
            return z;

    return masq[0];
};

export function get_forward_src_zones(dest_zone) {
    let cur = uci.cursor();
    if (!cur || !cur.load('firewall'))
        return null;

    let srcs = [];
    cur.foreach('firewall', 'forwarding', s => {
        if (s.dest != dest_zone || !s.src)
            return;
        /* Match fw4 boolean values. Forwardings are enabled by default. */
        let enabled = s.enabled ?? '1';
        if (index(['1', 'on', 'true', 'yes'], enabled) < 0)
            return;
        push(srcs, s.src);
    });

    return uniq(srcs);
};
