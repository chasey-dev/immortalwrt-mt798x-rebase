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
import * as _log from 'log';

const LOG_KEY = "mtwifi-cfg";

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

// read command output (trim all spaces, "\n"s)
export function read_pipe(cmd) {
	let fp = fs.popen(cmd, "r");
	if (!fp) return "";
	let txt = fp.read("all");
	fp.close();
	return txt ? replace(txt, /^[\s\n]+|[\s\n]+$/g, '') : "";
};

// merge arrays for MAC
export function array_unique(arr) {
	let seen = {};
	let out = [];
	for (let i in arr) {
		if (!seen[arr[i]]) {
			seen[arr[i]] = true;
			push(out, arr[i]);
		}
	}
	return out;
};

// lock helper
export function with_lock(func, lock_file, event) {
    // open file, create file if not exist
    let fd = fs.open(lock_file, "w+");

    if (!fd) {
        let err = fs.error();
        log.error(`[Lock] Could not open lock file ${lock_file}: ${err}`);
        // just exit with 1 here, netifd will handle retry
        exit(1);
    }

    // acquire exclusive lock
    let locked = fd.lock("x");

    if (!locked) {
        let err = fs.error();
        log.error(`[Lock] Failed to acquire exclusive lock on ${lock_file}: ${err}`);
        // close the file to ensure safety
        fd.close();
        exit(1);
    }
    
    log.debug(`[Lock] Exclusive lock acquired!!! (event: ${event})`);

    try {
        func();
    } catch (e) {
        log.error(`[Lock] Crashed during locked operation: ${e}\n${e.stacktrace}`);

        fd.close();
        exit(1);
    }

    // release the lock, 
    // but the lock will still be held if there is any forked process
    fd.close();
};