/* SPDX-License-Identifier: GPL-3.0-only */

'use strict';
'require form';
'require network';
'require poll';
'require rpc';
'require uci';
'require view';

let callQueueStats = rpc.declare({
	object: 'luci.eqos',
	method: 'getQueueStats',
	expect: { '': {} }
});

let callShortPkt = rpc.declare({
	object: 'luci.eqos',
	method: 'getShortPkt',
	expect: { '': {} }
});

/* ---------------------------------------------------------------- helpers */

function collectHostChoices(hosts) {
	var choices = { ip: [], ip6: [] };

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
	return selector === 'ip6' ? _('IPv6 address') : _('IPv4 address');
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

function priorityQid(prio, base) {
	return base + Math.floor((prio - 1) * 30 / 9);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('eqos'),
			network.getHostHints()
		]).then(function([eqos, hosts]) {
			return { hosts: hosts ? (hosts.hosts || {}) : {} };
		});
	},

	render: function(data) {
		var hostChoices = collectHostChoices(data.hosts);
		var m, s, o, gs;

		m = new form.Map('eqos', _('EQoS'),
			_('Network speed control service for MediaTek HNAT.'));
		m.tabbed = true;

		/* ------------------------------------------------- tab: HQoS settings */
		s = m.section(form.TypedSection, '_hqos', _('HQoS Settings'));
		s.anonymous = true;
		s.hidetitle = true;
		s.cfgsections = function() { return ['config']; };

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

		o = s.option(form.SectionValue, '__devices__', form.GridSection, 'device',
			_('Device rules'));
		gs = o.subsection;
		gs.addremove = true;
		gs.anonymous = true;
		gs.sortable = true;
		gs.nodescriptions = true;
		gs.handleAdd = function(ev) {
			var section_id = uci.add('eqos', 'device');

			uci.set('eqos', section_id, 'enabled', '1');
			uci.set('eqos', section_id, 'selector', 'ip');
			uci.set('eqos', section_id, 'download', '0');
			uci.set('eqos', section_id, 'upload', '0');
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		gs.tab('general', _('General Settings'));

		o = gs.taboption('general', form.Flag, 'enabled', _('Enable'));
		o.default = o.enabled;
		o.rmempty = false;
		o.editable = true;

		o = gs.taboption('general', form.Value, 'queue', _('Queue ID'),
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

		o = gs.option(form.DummyValue, '_match', _('Address'));
		o.textvalue = function(section_id) {
			var selector = selectorValue(section_id);
			var value = uci.get('eqos', section_id, selector);

			return value ? '%s: %s'.format(matchLabel(selector), value) : E('em', _('unspecified'));
		};

		o = gs.taboption('general', form.Value, 'download', _('Download'),
			_('Maximum rate in Mbit/s. Use 0 for no limit.'));
		o.datatype = 'and(ufloat,min(0),max(1000))';
		o.rmempty = false;
		o.cfgvalue = rateCfgvalue;
		o.write = rateWrite;
		o.textvalue = rateText;

		o = gs.taboption('general', form.Value, 'upload', _('Upload'),
			_('Maximum rate in Mbit/s. Use 0 for no limit.'));
		o.datatype = 'and(ufloat,min(0),max(1000))';
		o.rmempty = false;
		o.cfgvalue = rateCfgvalue;
		o.write = rateWrite;
		o.textvalue = rateText;

		o = gs.taboption('general', form.ListValue, 'selector', _('Type'));
		o.modalonly = true;
		o.default = 'ip';
		o.rmempty = false;
		o.value('ip', _('IPv4 address'));
		o.value('ip6', _('IPv6 address'));
		o.cfgvalue = function(section_id) { return selectorValue(section_id); };
		o.write = function(section_id, value) {
			uci.set('eqos', section_id, 'selector', value);

			if (value !== 'ip')
				uci.unset('eqos', section_id, 'ip');

			if (value !== 'ip6')
				uci.unset('eqos', section_id, 'ip6');
		};

		o = gs.taboption('general', form.Value, 'ip', _('IPv4 address'));
		o.modalonly = true;
		o.datatype = 'ip4addr("nomask")';
		o.rmempty = false;
		o.depends('selector', 'ip');
		o.validate = uniqueAddress;
		addChoices(o, hostChoices.ip);

		o = gs.taboption('general', form.Value, 'ip6', _('IPv6 address'));
		o.modalonly = true;
		o.datatype = 'ip6addr("nomask")';
		o.rmempty = false;
		o.depends('selector', 'ip6');
		o.validate = uniqueAddress;
		addChoices(o, hostChoices.ip6);

		/* ---------------------------------------------------- tab: priority */
		s = m.section(form.TypedSection, '_priority', _('Packet priority settings'));
		s.anonymous = true;
		s.hidetitle = true;
		s.cfgsections = function() { return ['config']; };

		o = s.option(form.Flag, 'short_pkt_priority', _('Short-packet priority'),
			_('Recognizes short flows (TCP ACK / small UDP) in hardware before offload and puts them into the priority queue below. Tune it with "Short-flow length" and "Priority queue ID". Works on any port, including direct 2.5G.'));
		o.default = o.disabled;
		o.rmempty = false;

		o = s.option(form.Value, 'short_pkt_len', _('Short-flow length'),
			_('Maximum average packet size (16-128 bytes) treated as a short flow.'));
		o.depends('short_pkt_priority', '1');
		o.datatype = 'and(uinteger,min(16),max(128))';
		o.placeholder = '64';
		o.write = integerWrite;

		o = s.option(form.Value, 'short_pkt_qid', _('Priority queue ID'),
			_('Short-packet flows (TCP ACK / small UDP) are steered to this hardware queue. On MTK hardware larger queue numbers mean higher priority. To give it a speed limit, add a Device rule below and enter the same number as its Queue ID: Queue ID N limits upload queue N and download queue N+31. Default 14.'));
		o.depends('short_pkt_priority', '1');
		o.datatype = 'and(uinteger,min(1),max(62))';
		o.placeholder = '14';
		o.write = integerWrite;

		o = s.option(form.SectionValue, '__rules__', form.GridSection, 'priority_rule',
			_('Priority rules'));
		gs = o.subsection;
		gs.addremove = true;
		gs.anonymous = true;
		gs.nodescriptions = true;
		gs.handleAdd = function(ev) {
			var section_id = uci.add('eqos', 'priority_rule');
			m.addedSection = section_id;
			return this.renderMoreOptionsModal(section_id);
		};

		o = gs.option(form.Flag, 'enabled', _('Enable'));
		o.default = o.enabled;
		o.rmempty = false;
		o.editable = true;

		o = gs.option(form.ListValue, 'protocol', _('Protocol'));
		o.value('', _('Any'));
		o.value('tcp', 'TCP');
		o.value('udp', 'UDP');
		o.value('icmp', 'ICMP');
		o.default = '';

		o = gs.option(form.Value, 'sport', _('Source port'),
			_('Optional, e.g. 443 or 27000-27030.'));
		o.datatype = 'or(port,portrange)';
		o.rmempty = true;

		o = gs.option(form.Value, 'dport', _('Dest port'),
			_('Optional, e.g. 53 or 27000-27030.'));
		o.datatype = 'or(port,portrange)';
		o.rmempty = true;

		o = gs.option(form.Value, 'dscp', _('DSCP'),
			_('Optional DSCP value 0-63, e.g. 46 for EF.'));
		o.datatype = 'and(uinteger,min(0),max(63))';
		o.rmempty = true;

		o = gs.option(form.ListValue, 'direction', _('Direction'));
		o.value('up', _('Upload'));
		o.value('down', _('Download'));
		o.value('both', _('Both'));
		o.default = 'both';

		o = gs.option(form.ListValue, 'priority', _('Priority'),
			_('1 lowest .. 10 highest. Maps to upload queue 1-31 / download queue 32-62. Custom rules take precedence over short-packet auto detection.'));
		for (var i = 1; i <= 10; i++)
			o.value(String(i), String(i));
		o.default = '5';
		o.rmempty = false;

		o = gs.option(form.DummyValue, '_qidmap', _('Queue map'));
		o.textvalue = function(section_id) {
			var p = Number(uci.get('eqos', section_id, 'priority') || 5);
			var d = uci.get('eqos', section_id, 'direction') || 'both';
			var parts = [];
			if (d === 'up' || d === 'both')
				parts.push('%s: %s'.format(_('Upload'), priorityQid(p, 1)));
			if (d === 'down' || d === 'both')
				parts.push('%s: %s'.format(_('Download'), priorityQid(p, 32)));
			return parts.join(' / ');
		};
		o.validate = function(section_id, value) {
			var prio = Number(value);
			var dir = uci.get('eqos', section_id, 'direction') || 'both';
			var qidUp = priorityQid(prio, 1);
			var qidDl = priorityQid(prio, 32);
			var i;

			var rules = uci.sections('eqos', 'priority_rule');
			for (i = 0; i < rules.length; i++) {
				if (rules[i]['.name'] === section_id || rules[i].enabled === '0')
					continue;

				var p = Number(rules[i].priority || 5);
				var d = rules[i].direction || 'both';
				if ((dir === 'up' || dir === 'both') && (d === 'up' || d === 'both') &&
				    priorityQid(p, 1) === qidUp)
					return _('Upload queue %s is already used by another rule.').format(qidUp);
				if ((dir === 'down' || dir === 'both') && (d === 'down' || d === 'both') &&
				    priorityQid(p, 32) === qidDl)
					return _('Download queue %s is already used by another rule.').format(qidDl);
			}

			if (uci.get('eqos', 'config', 'short_pkt_priority') === '1') {
				var sp = Number(uci.get('eqos', 'config', 'short_pkt_qid') || 14);
				if ((dir === 'up' || dir === 'both') && qidUp === sp)
					return _('Upload queue %s conflicts with the short-packet priority queue.').format(sp);
			}

			var devices = uci.sections('eqos', 'device');
			for (i = 0; i < devices.length; i++) {
				if (devices[i].enabled === '0')
					continue;
				var slot = Number(devices[i].queue || devices[i].comment);
				if (slot >= 1 && slot <= 31) {
					if ((dir === 'up' || dir === 'both') && qidUp === slot)
						return _('Upload queue %s is already used by a device rule.').format(slot);
					if ((dir === 'down' || dir === 'both') && qidDl === 31 + slot)
						return _('Download queue %s is already used by a device rule.').format(31 + slot);
				}
			}

			return true;
		};

		/* ----------------------------------------------------- tab: monitor */
		s = m.section(form.TypedSection, '_monitor', _('Queue Monitor'));
		s.anonymous = true;
		s.hidetitle = true;
		s.cfgsections = function() { return ['_monitor']; };
		s.renderContents = function() {
			return Promise.resolve(form.TypedSection.prototype.renderContents.apply(this, arguments))
				.then(function(sectionEl) {
					sectionEl.appendChild(buildMonitor());
					return sectionEl;
				});
		};

		return m.render().then(function(nodes) {
			poll.add(async function() {
				let [sq, qs] = await Promise.all([
					L.resolveDefault(callShortPkt(), {}),
					L.resolveDefault(callQueueStats(), {})
				]);

				let infoEl = document.getElementById('eqos-qmon-info');
				if (infoEl) {
					let parts = [
						'%s: %s'.format(_('Mode'), sq.qos || '-'),
						'%s: %s'.format(_('Len'), sq.len || '-'),
						'%s: %s'.format(_('Queue'), sq.qid || '-'),
						'%s: %s'.format(_('QoS'), sq.qos_toggle || '-'),
						'%s: %s'.format(_('Path'), sq.path || '-')
					];
					L.dom.content(infoEl, E('span', {}, parts.join('  |  ')));
				}

				let body = document.getElementById('eqos-qmon-body');
				if (!body)
					return;

				let activeOnly = true;
				let activeEl = document.getElementById('eqos-qmon-active');
				if (activeEl)
					activeOnly = activeEl.checked;

				L.dom.content(body, null);
				let rows = [];
				for (let q in (qs.queues || {})) {
					let qq = qs.queues[q];
					if (activeOnly && !qq.packets && !qq.bytes && !qq.bps && !qq.pps)
						continue;
					rows.push(E('tr', { 'class': 'tr' }, [
						E('td', {}, q),
						E('td', {}, qs.has_counters && qq.bps ? '%s Mbit/s'.format((qq.bps / 1e6).toFixed(2)) : '-'),
						E('td', {}, qs.has_counters ? String(qq.pps || 0) : '-'),
						E('td', {}, qs.has_counters ? String(qq.packets || 0) : '-')
					]));
				}
				if (!rows.length)
					rows.push(E('tr', { 'class': 'tr' }, [
						E('td', { 'colspan': 5 }, E('em', {}, _('No data')))
					]));
				L.dom.content(body, rows);
			});

			return nodes;
		});

		function buildMonitor() {
			let info = E('div', { 'id': 'eqos-qmon-info' },
				E('em', {}, _('Collecting data...')));

			let toggle = E('label', { 'class': 'eqos-qmon-toggle' },
				[ E('input', { 'type': 'checkbox', 'id': 'eqos-qmon-active', 'checked': true }),
				  ' ', _('Active queues only') ]);

			let table = E('table', { 'class': 'table eqos-qmon-table' }, [
				E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'left' }, _('Queue')),
					E('th', { 'class': 'left' }, _('Rate')),
					E('th', { 'class': 'left' }, _('Packets/s')),
					E('th', { 'class': 'left' }, _('Packets'))
				]),
				E('tbody', { 'id': 'eqos-qmon-body' })
			]);

			return E('div', {}, [
				E('style', {}, '.eqos-qmon-table th,.eqos-qmon-table td{padding:1px 8px;font-size:12px;line-height:1.35}'),
				info,
				toggle,
				table
			]);
		}
	}
});
