'use strict';
'require form';
'require network';
'require uci';
'require view';

function collectHostChoices(hosts) {
	var choices = {
		ip: [],
		ip6: []
	};

	for (var host in hosts) {
		var ipaddrs = L.toArray(hosts[host].ipaddrs || hosts[host].ipv4);
		var ip6addrs = L.toArray(hosts[host].ip6addrs || hosts[host].ipv6);
		var name = hosts[host].name;

		for (var i = 0; i < ipaddrs.length; i++)
			choices.ip.push([ ipaddrs[i], name ? '%s (%s)'.format(name, ipaddrs[i]) : ipaddrs[i] ]);

		for (var j = 0; j < ip6addrs.length; j++)
			choices.ip6.push([ ip6addrs[j], name ? '%s (%s)'.format(name, ip6addrs[j]) : ip6addrs[j] ]);
	}

	return choices;
}

function addChoices(option, choices) {
	for (var i = 0; i < choices.length; i++)
		option.value(choices[i][0], choices[i][1]);
}

function selectorValue(section_id) {
	var selector = uci.get('eqos', section_id, 'selector');

	if (selector === 'ip' || selector === 'ip6')
		return selector;

	if (uci.get('eqos', section_id, 'ip6'))
		return 'ip6';

	return 'ip';
}

function matchLabel(selector) {
	if (selector === 'ip6')
		return _('IPv6 address');

	return _('IPv4 address');
}

function rateCfgvalue(section_id) {
	return String(Number(uci.get('eqos', section_id, this.option) || 0) / 1000);
}

function rateWrite(section_id, value) {
	uci.set('eqos', section_id, this.option, String(Math.round(Number(value) * 1000)));
}

function integerWrite(section_id, value) {
	uci.set('eqos', section_id, this.option, String(Number(value)));
}

function uniqueAddress(section_id, value) {
	var sections = uci.sections('eqos', 'device');

	for (var i = 0; i < sections.length; i++) {
		if (sections[i]['.name'] === section_id || sections[i].enabled === '0')
			continue;

		if (sections[i][this.option] === value)
			return _('This value is already in use.');
	}

	return true;
}

function rateText(section_id) {
	var value = this.cfgvalue(section_id) || '0';

	return value === '0' ? _('unlimited') : '%s Mbit/s'.format(value);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('eqos'),
			network.getHostHints()
		]);
	},

	render: function(data) {
		var hosts = data[1] ? data[1].hosts || {} : {};
		var hostChoices = collectHostChoices(hosts);
		var m, s, o;

		m = new form.Map('eqos', _('EQoS'),
			_('Network speed control service for MediaTek HNAT.'));

		s = m.section(form.NamedSection, 'config', 'eqos', _('Settings'));
		s.anonymous = true;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = o.disabled;
		o.rmempty = false;

		o = s.option(form.Value, 'download', '%s (Mbit/s)'.format(_('Download')),
			_('Total download bandwidth.'));
		o.datatype = 'and(uinteger,min(1),max(1000))';
		o.rmempty = false;
		o.write = integerWrite;

		o = s.option(form.Value, 'upload', '%s (Mbit/s)'.format(_('Upload')),
			_('Total upload bandwidth.'));
		o.datatype = 'and(uinteger,min(1),max(1000))';
		o.rmempty = false;
		o.write = integerWrite;

		s = m.section(form.GridSection, 'device', _('Device rules'));
		s.addremove = true;
		s.anonymous = true;
		s.sortable = true;
		s.nodescriptions = true;
		s.handleAdd = function(ev) {
			var section_id = uci.add('eqos', 'device');

			uci.set('eqos', section_id, 'enabled', '1');
			uci.set('eqos', section_id, 'selector', 'ip');
			uci.set('eqos', section_id, 'download', '0');
			uci.set('eqos', section_id, 'upload', '0');
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		s.tab('general', _('General Settings'));

		o = s.taboption('general', form.Flag, 'enabled', _('Enable'));
		o.default = o.enabled;
		o.rmempty = false;
		o.editable = true;

		o = s.taboption('general', form.Value, 'queue', _('Queue ID'),
			_('Values 1-31 use HNAT HQoS. Values 32 and above use software shaping.'));
		o.datatype = 'and(uinteger,min(1),max(65535))';
		o.placeholder = '1';
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return uci.get('eqos', section_id, 'queue') ||
				uci.get('eqos', section_id, 'comment');
		};
		o.write = function(section_id, value) {
			uci.set('eqos', section_id, 'queue', String(Number(value)));
			uci.unset('eqos', section_id, 'comment');
		};
		o.validate = function(section_id, value) {
			var sections = uci.sections('eqos', 'device');

			for (var i = 0; i < sections.length; i++) {
				if (sections[i]['.name'] === section_id || sections[i].enabled === '0')
					continue;

				if (Number(sections[i].queue || sections[i].comment) === Number(value))
					return _('This value is already in use.');
			}

			return true;
		};

		o = s.option(form.DummyValue, '_match', _('Address'));
		o.textvalue = function(section_id) {
			var selector = selectorValue(section_id);
			var value = uci.get('eqos', section_id, selector);

			return value ? '%s: %s'.format(matchLabel(selector), value) : E('em', _('unspecified'));
		};

		o = s.taboption('general', form.Value, 'download', _('Download'),
			_('Maximum rate in Mbit/s. Use 0 for no limit.'));
		o.datatype = 'and(ufloat,min(0),max(1000))';
		o.rmempty = false;
		o.cfgvalue = rateCfgvalue;
		o.write = rateWrite;
		o.textvalue = rateText;

		o = s.taboption('general', form.Value, 'upload', _('Upload'),
			_('Maximum rate in Mbit/s. Use 0 for no limit.'));
		o.datatype = 'and(ufloat,min(0),max(1000))';
		o.rmempty = false;
		o.cfgvalue = rateCfgvalue;
		o.write = rateWrite;
		o.textvalue = rateText;

		o = s.taboption('general', form.ListValue, 'selector', _('Type'));
		o.modalonly = true;
		o.default = 'ip';
		o.rmempty = false;
		o.value('ip', _('IPv4 address'));
		o.value('ip6', _('IPv6 address'));
		o.cfgvalue = function(section_id) {
			return selectorValue(section_id);
		};
		o.write = function(section_id, value) {
			uci.set('eqos', section_id, 'selector', value);

			if (value !== 'ip')
				uci.unset('eqos', section_id, 'ip');

			if (value !== 'ip6')
				uci.unset('eqos', section_id, 'ip6');
		};

		o = s.taboption('general', form.Value, 'ip', _('IPv4 address'));
		o.modalonly = true;
		o.datatype = 'ip4addr("nomask")';
		o.rmempty = false;
		o.depends('selector', 'ip');
		o.validate = uniqueAddress;
		addChoices(o, hostChoices.ip);

		o = s.taboption('general', form.Value, 'ip6', _('IPv6 address'));
		o.modalonly = true;
		o.datatype = 'ip6addr("nomask")';
		o.rmempty = false;
		o.depends('selector', 'ip6');
		o.validate = uniqueAddress;
		addChoices(o, hostChoices.ip6);

		return m.render();
	}
});
