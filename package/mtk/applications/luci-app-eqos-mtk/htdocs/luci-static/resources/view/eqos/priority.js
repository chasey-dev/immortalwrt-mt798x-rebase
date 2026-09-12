'use strict';
'require form';
'require uci';
'require view';

function integerWrite(section_id, value) {
	uci.set('eqos', section_id, this.option, String(Number(value)));
}

function priorityQid(prio, base) {
	return base + Math.floor((prio - 1) * 30 / 9);
}

return view.extend({
	load: function() {
		return uci.load('eqos');
	},

	render: function(data) {
		var m, s, o;

		m = new form.Map('eqos', _('Packet priority settings'),
			_('Packet priority settings for MediaTek HNAT.'));

		s = m.section(form.NamedSection, 'config', 'eqos', _('Packet priority settings'));
		s.anonymous = true;

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

		s = m.section(form.GridSection, 'priority_rule', _('Priority rules'));
		s.addremove = true;
		s.anonymous = true;
		s.nodescriptions = true;
		s.handleAdd = function(ev) {
			var section_id = uci.add('eqos', 'priority_rule');
			m.addedSection = section_id;
			return this.renderMoreOptionsModal(section_id);
		};

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = o.enabled;
		o.rmempty = false;
		o.editable = true;

		o = s.option(form.ListValue, 'protocol', _('Protocol'));
		o.value('', _('Any'));
		o.value('tcp', 'TCP');
		o.value('udp', 'UDP');
		o.value('icmp', 'ICMP');
		o.default = '';

		o = s.option(form.Value, 'sport', _('Source port'),
			_('Optional, e.g. 443 or 27000-27030.'));
		o.datatype = 'or(port,portrange)';
		o.rmempty = true;

		o = s.option(form.Value, 'dport', _('Dest port'),
			_('Optional, e.g. 53 or 27000-27030.'));
		o.datatype = 'or(port,portrange)';
		o.rmempty = true;

		o = s.option(form.Value, 'dscp', _('DSCP'),
			_('Optional DSCP value 0-63, e.g. 46 for EF.'));
		o.datatype = 'and(uinteger,min(0),max(63))';
		o.rmempty = true;

		o = s.option(form.ListValue, 'direction', _('Direction'));
		o.value('up', _('Upload'));
		o.value('down', _('Download'));
		o.value('both', _('Both'));
		o.default = 'both';

		o = s.option(form.ListValue, 'priority', _('Priority'),
			_('1 lowest .. 10 highest. Maps to upload queue 1-31 / download queue 32-62. Custom rules take precedence over short-packet auto detection.'));
		for (var i = 1; i <= 10; i++)
			o.value(String(i), String(i));
		o.default = '5';
		o.rmempty = false;

		o = s.option(form.DummyValue, '_qidmap', _('Queue map'));
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

		return m.render();
	}
});
