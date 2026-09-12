'use strict';
'require poll';
'require rpc';
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

return view.extend({
	render: function() {
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

		return E('div', { 'class': 'cbi-section' }, [
			E('style', {}, '.eqos-qmon-table th,.eqos-qmon-table td{padding:1px 8px;font-size:12px;line-height:1.35}'),
			info,
			toggle,
			table
		]);
	}
});
