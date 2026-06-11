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
import * as fs from 'fs';

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

/**
 * Return the first-boot UCI defaults for one band.
 *
 * The defaults intentionally create an open AP so users can connect first and
 * change SSID/security from LuCI or UCI later.
 *
 * @param {string} band - mtwifi band name: 2g, 5g, or 6g.
 * @returns {Object} Default htmode, htbsscoex, and ssid values.
 */
function get_band_defaults(band) {
    if (band == "2g") {
        return { htmode: "EHT40", htbsscoex: 1, ssid: "ImmortalWrt-2.4G" };
    } else if (band == "5g") {
        return { htmode: "EHT160", htbsscoex: 0, ssid: "ImmortalWrt-5G" };
    } else {
        return { htmode: "EHT160", htbsscoex: 0, ssid: "ImmortalWrt-6G" };
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
let mtwifi_phys = {};
let mtwifi_paths = {};
let board = json(fs.readfile("/etc/board.json") || "{}");

// iter by ordered devnames, preventing vif disorder in UCI cfgs
for (let devname in all_devnames) {
    let cur_dev = all_devs[devname];
    let phy = driver.phy_from_ifname(cur_dev.main_ifname);

    if (phy) {
        mtwifi_phys[phy] = true;
        if (board.wlan?.[phy]?.path)
            mtwifi_paths[board.wlan[phy].path] = true;
    }

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

    let defs = get_band_defaults(band);

    // create wifi-device node
    cursor.set("wireless", devname, "wifi-device");
    
    // call helper functions to batch set properties
    set_section_options("wireless", devname, {
        "type": "mtwifi",
        "phy": phy,
        "band": band,
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

let mac80211_devs = {};
cursor.foreach("wireless", "wifi-device", function(sec) {
    if (sec.type != "mac80211")
        return;

    if (!mtwifi_phys[sec.phy] && !mtwifi_paths[sec.path])
        return;

    mac80211_devs[sec[".name"]] = true;
});

let mac80211_ifaces = [];
cursor.foreach("wireless", "wifi-iface", function(sec) {
    if (!mac80211_devs[sec.device])
        return;

    push(mac80211_ifaces, sec[".name"]);
});

for (let iface in mac80211_ifaces) {
    cursor.delete("wireless", iface);
    need_commit = true;
}

for (let devname in mac80211_devs) {
    cursor.delete("wireless", devname);
    need_commit = true;
}

l1.close();

// commit in one shot
if (need_commit) {
    cursor.commit("wireless");
}
