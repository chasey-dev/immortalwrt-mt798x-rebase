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

export function zmap(state) {
    let m = {};
    for (let z in (state.zones || []))
        m[z.name] = z;
    return m;
};

export function zone_has_iface(z, ifname) {
    let nets = z.network || [];
    for (let n in nets)
        if (n.val == ifname || n.device == ifname)
            return true;
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

export function forward_src_zones(dest_zone) {
    let cur = uci.cursor();
    cur.load('firewall');

    let srcs = [];
    cur.foreach('firewall', 'forwarding', s => {
        let en = (s.enabled == null) ? '1' : s.enabled;
        if (en != '0' && s.dest == dest_zone && s.src)
            push(srcs, s.src);
    });

    return uniq(srcs);
};

export function pick_best(cands, prefer) {
    for (let p in prefer)
        for (let c in cands)
            if (c == p) return c;
    return length(cands) ? cands[0] : null;
};
