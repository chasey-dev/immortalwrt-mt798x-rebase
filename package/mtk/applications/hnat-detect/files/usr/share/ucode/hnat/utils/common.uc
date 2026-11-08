/*
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
import * as _log from 'log';

const LOG_KEY = "hnat-detect";

_log.openlog(LOG_KEY, _log.LOG_PID, _log.LOG_USER);

export const log = {
    error: function(msg) {
        _log.syslog(_log.LOG_ERR, "%s", msg);
    },

	warn: function(msg) {
        _log.syslog(_log.LOG_WARNING, "%s", msg);
    },

    notice: function(msg) {
        _log.syslog(_log.LOG_NOTICE, "%s", msg);
    },
    
    info: function(msg) {
        _log.syslog(_log.LOG_INFO, "%s", msg);
    },

	debug: function(msg) {
        _log.syslog(_log.LOG_DEBUG, "%s", msg);
    }
};

export function read_first_token(path) {
    let text = fs.readfile(path);
    if (text == null)
        return null;
    return match(trim(text), /^(\S+)/)?.[1];
};

export function merge(...arrays) {
    let r = [];
    for (let a in arrays)
        push(r, ...a);
    return r;
};
