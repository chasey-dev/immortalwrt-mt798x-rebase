#!/usr/bin/ucode

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

import * as uci from 'uci';
import * as l1parser from 'l1parser';

import * as driver from 'mtwifi.driver';

// check if driver is installed in kmods
if (!driver.is_kmod()) {
    exit(1);
}

let cursor = uci.cursor();
// load uci config
cursor.load("wireless");

let l1 = l1parser.open();

// unordered object
let all_devs = l1.getall();
// get devnames listed by order
let all_devnames = l1.list();

// helper to set default settings by band
function get_band_defaults(band) {
    if (band == "2g") {
        return { htmode: "HE40", htbsscoex: 1, ssid: "ImmortalWrt-2.4G" };
    } else if (band == "5g") {
        return { htmode: "HE160", htbsscoex: 0, ssid: "ImmortalWrt-5G" };
    } else {
        return { htmode: "HE160", htbsscoex: 0, ssid: "ImmortalWrt-6G" };
    }
}

// helper to batch setting properties
function set_section_options(config, section, values) {
    for (let k, v in values) {
        // uci.set(config, section, option, value)
        cursor.set(config, section, k, v);
    }
}

let need_commit = false;

// iter by ordered devnames, preventing vif disorder in UCI cfgs
for (let devname in all_devnames) {
    let cur_dev = all_devs[devname];
    // returns null if not exist
    let type = cursor.get("wireless", devname);
    
    // if node exists, skip
    if (type == "wifi-device") {
        continue;
    }

    let subidx = int(cur_dev.subidx);
    let band = cur_dev.band;
    if (!band || band == "nil") {
        band = (subidx == 1) ? "2g" : "5g";
    }

    let dbdc_main = (subidx == 1) ? 1 : 0;
    let defs = get_band_defaults(band);

    // create wifi-device node
    cursor.set("wireless", devname, "wifi-device");
    
    // call helper functions to batch set properties
    set_section_options("wireless", devname, {
        "type": "mtwifi",
        "phy": cur_dev.main_ifname,
        "band": band,
        "dbdc_main": dbdc_main,
        "channel": "auto",
        "txpower": 100,
        "htmode": defs.htmode,
        "country": "CN",
        "mu_beamformer": 1,
        "noscan": defs.htbsscoex,
        "serialize": 1
    });

    // create wifi-iface node
    let iface_name = "default_" + devname;
    cursor.set("wireless", iface_name, "wifi-iface");
    
    // call helper functions to batch set properties
    set_section_options("wireless", iface_name, {
        "device": devname,
        "network": "lan",
        "mode": "ap",
        "ssid": defs.ssid,
        "encryption": "none"
    });

    need_commit = true;
}

l1.close();

// commit in one shot
if (need_commit) {
    cursor.commit("wireless");
}