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

import * as fs from 'fs';

const dat_defs = "dat-defs";
const device = "wireless.device";
const iface = "wireless.iface";
const vlan = "wireless.vlan";
const station = "wireless.station";

const schema_path = "/usr/share/schema/mtwifi";

const dat_defs_path = `${schema_path}/${dat_defs}.json`;
const device_path = `${schema_path}/${device}.json`;
const iface_path = `${schema_path}/${iface}.json`;
const vlan_path = `${schema_path}/${vlan}.json`;
const station_path = `${schema_path}/${station}.json`;

// parse DAT config defaults
export const defs = json(fs.readfile(dat_defs_path));

// parse UCI config schema
const device_schema = json(fs.readfile(device_path));
const iface_schema = json(fs.readfile(iface_path));
const vlan_schema = json(fs.readfile(vlan_path));
const station_schema = json(fs.readfile(station_path));

export const schemas = {
    device:     device_schema.properties,
    iface:      iface_schema.properties,
    vlan:       vlan_schema.properties,
    station:    station_schema.properties
};
