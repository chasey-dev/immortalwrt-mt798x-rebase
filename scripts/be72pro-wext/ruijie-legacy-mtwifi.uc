'use strict';

import * as datconf from 'datconf';
import * as driver from 'mtwifi.driver';
import * as converter from 'mtwifi.converter';
import * as netifd from 'mtwifi.netifd';

const PERSONAL_ENCRYPTION = [
	'none', 'psk2', 'psk2+ccmp', 'psk2+aes', 'sae', 'sae+ccmp', 'sae-mixed'
];

function is_dat_text(value) {
	return type(value) == 'string' && index(value, '\r') < 0 &&
		index(value, '\n') < 0 && index(value, chr(0)) < 0;
}

function validate_ap(config) {
	if (config.mode != 'ap' || config.mlo || config.macfilter in ['allow', 'deny'])
		return 'LEGACY_AP_CONFIG_UNSUPPORTED';
	if (!is_dat_text(config.ssid) || !length(config.ssid) || length(config.ssid) > 32)
		return 'LEGACY_INVALID_SSID';
	if (!(config.encryption in PERSONAL_ENCRYPTION))
		return 'LEGACY_ENCRYPTION_UNSUPPORTED';
	if (config.encryption == 'none')
		return null;
	let key = config.key || '';
	if (!is_dat_text(key) || length(key) < 8 || length(key) > 64 ||
	    (length(key) == 64 && (index(config.encryption, 'psk2') != 0 || !match(key, /^[0-9a-fA-F]{64}$/))))
		return 'LEGACY_INVALID_PSK';
	return null;
}

function set_link(ifname, state) {
	return system(['ip', 'link', 'set', 'dev', ifname, state], 30000) == 0;
}

export function setup(data, dev) {
	let interfaces = [];
	for (let id, iface in data.interfaces) {
		let error = validate_ap(iface.config);
		if (error)
			return error;
		push(interfaces, { id, ifname: iface.mtwifi_ifname });
	}
	if (!length(interfaces) || interfaces[0].ifname != dev.main_ifname)
		return 'LEGACY_PRIMARY_AP_REQUIRED';

	let profile = datconf.open(dev.profile_path);
	if (!profile)
		return 'LEGACY_DAT_OPEN_FAILED';
	let update = converter.convert(data);
	let values = update.values;
	// The vendor driver creates MBSSID slots on the first open.
	values.BssidNum = length(interfaces) > 4 ? length(interfaces) : 4;
	values.AuthMode = replace(values.AuthMode, /WPA2PSKMIXWPA3PSK/g, 'WPA2PSKWPA3PSK');
	values.MacAddress ??= profile.get('MacAddress_Default');

	for (let ifname in driver.scan_related_vifs(dev)) {
		if (!set_link(ifname, 'down')) {
			profile.close();
			return 'LEGACY_LINK_DOWN_FAILED';
		}
	}

	for (let key in update.unset)
		profile.unset(key);
	profile.merge(values);
	profile.commit();
	profile.close();

	profile = datconf.open(dev.profile_path);
	if (!profile)
		return 'LEGACY_DAT_WRITE_FAILED';
	let written = profile.get('SSID1') == values.SSID1 &&
		profile.get('AuthMode') == values.AuthMode &&
		(!values.WPAPSK1 || profile.get('WPAPSK1') == values.WPAPSK1);
	profile.close();
	if (!written)
		return 'LEGACY_DAT_WRITE_FAILED';

	let opened = [];
	for (let iface in interfaces) {
		if (!driver.wait_for_iface(iface.ifname) || !set_link(iface.ifname, 'up')) {
			while (length(opened))
				set_link(pop(opened), 'down');
			return 'LEGACY_LINK_UP_FAILED';
		}
		push(opened, iface.ifname);
	}
	for (let iface in interfaces)
		netifd.set_vif(iface.id, iface.ifname);
	netifd.set_up();
	return null;
}
