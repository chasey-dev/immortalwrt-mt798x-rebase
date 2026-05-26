/* Copyright (C) 2022 ImmortalWrt.org */

'use strict';
'require form';
'require fs';
'require poll';
'require rpc';
'require uci';
'require view';

let callSystemFeatures = rpc.declare({
	object: 'luci.turboacc',
	method: 'getSystemFeatures',
	expect: { '': {} }
});

let callFastPathStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFastPathStat',
	expect: { '': {} }
});

let callFullConeStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeStat',
	expect: { '': {} }
});

let callTCPCCAStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getTCPCCAStat',
	expect: { '': {} }
});

let callMTKPPEStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getMTKPPEStat',
	expect: { '': {} }
});

function renderProgressBar(value, max, byte) {
	let vn = parseInt(value) || 0,
		mn = parseInt(max) || 100,
		pc = Math.floor((100 / mn) * vn),
		text_val = byte ? String.format('%1024.2mB', value) : value,
		text_max = byte ? String.format('%1024.2mB', max) : max;

	return E('div', {
		'class': 'cbi-progressbar',
		'title': '%s / %s (%d%%)'.format(text_val, text_max, pc)
	}, E('div', { 'style': 'width:%.2f%%'.format(pc) }));
}

function renderStatusItem(stat) {
	if (!stat || !stat.type) {
		return E('em', { 'style': 'color:red; font-weight:bold' }, _('Disabled'));
	}

	if (stat.type.includes(' / ')) {
		let parts = stat.type.split(' / ');
		let elems = [];

		elems.push(E('span', { 'style': 'color:green; font-weight:bold' }, parts[0]));

		for (let i = 1; i < parts.length; i++) {
			elems.push(document.createTextNode(' / '));
			elems.push(E('span', { 'style': 'color:red; font-weight:bold' }, parts[i]));
		}

		return E('em', {}, elems);
	}

	return E('em', { 'style': 'color:green; font-weight:bold' }, stat.type);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('turboacc'),
			L.resolveDefault(callSystemFeatures(), {}),
			L.resolveDefault(callMTKPPEStat(), {})
		]);
	},

	render: function(data) {
		let features = data[1];
		let ppe_stats = data[2];

		/* only check ppe when mtk_hnat is enabled */
		let current_fastpath = uci.get('turboacc', 'config', 'fastpath');
		let has_ppe = (current_fastpath == 'mediatek_hnat') && ppe_stats && ppe_stats['PPE num'];

		let m, s, o;

		m = new form.Map('turboacc', _('TurboACC settings'),
			_('Open source flow offloading engine (fast path or hardware NAT).'));

		s = m.section(form.TypedSection);
		s.anonymous = true;
		s.render = function () {
			/* Basic Status */
			let table_rows = [
				E('tr', {}, [
					E('td', { 'class': 'tdLeft', 'width': '33%' }, _('FastPath Engine')),
					E('td', { 'id': 'fastpath_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'class': 'tdLeft' }, _('Full Cone NAT')),
					E('td', { 'id': 'fullcone_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'class': 'tdLeft' }, _('TCP CCA')),
					E('td', { 'id': 'tcpcca_state' }, E('em', {}, _('Collecting data...')))
				])
			];

			/* PPE Stats */
			if (has_ppe) {
				let ppe_num = parseInt(ppe_stats['PPE num']);
				for (let i = 0; i < ppe_num; i++) {
					let ppe_key = 'PPE' + i;
					if (ppe_stats[ppe_key]) {
						table_rows.push(E('tr', {}, [
							E('td', { 'class': 'tdLeft' }, `${ppe_key} ` + _('Bind Entries')),
							E('td', { 'id': `ppe${i}_entry` }, E('em', {}, _('Collecting data...')))
						]));
					}
				}
			}

			let acc_status = E('table', { 'class': 'table' }, table_rows);

			poll.add(async function () {
				let tasks = [
					L.resolveDefault(callFastPathStat(), {}),
					L.resolveDefault(callFullConeStat(), {}),
					L.resolveDefault(callTCPCCAStat(), {})
				];

				if (has_ppe) {
					tasks.push(L.resolveDefault(callMTKPPEStat(), {}));
				}

				let res = await Promise.all(tasks);

				const dom_ids = ['fastpath_state', 'fullcone_state', 'tcpcca_state'];
				for (let i = 0; i < 3; i++) {
					let el = document.getElementById(dom_ids[i]);
					if (el) {
						L.dom.content(el, renderStatusItem(res[i]));
					}
				}

				if (has_ppe) {
					let mtk_res = res[3];
					let ppe_num = parseInt(mtk_res['PPE num'] || 0);
					for (let j = 0; j < ppe_num; j++) {
						let ppe_key = 'PPE' + j;
						let ppe_bar = document.getElementById(`ppe${j}_entry`);

						if (ppe_bar && mtk_res[ppe_key]) {
							let bind_num = mtk_res[ppe_key]['BIND state num'];
							let total_num = mtk_res[ppe_key]['entry num'];
							L.dom.content(ppe_bar, renderProgressBar(bind_num, total_num));
						}
					}
				}
			});

			return E('div', { 'class': 'cbi-section' }, [
				E('h3', {}, _('Acceleration Status')),
				acc_status
			]);
		};

		/* Mark user edited */
		s = m.section(form.NamedSection, 'global', 'turboacc');
		o = s.option(form.HiddenValue, 'set');
		o.default = '1'; 
		o.forcewrite = true;

		s = m.section(form.NamedSection, 'config', 'turboacc');

		o = s.option(form.ListValue, 'fastpath', _('Fastpath engine'),
			_('The offloading engine for routing/NAT.'));
		o.value('disabled', _('Disable'));

		if (features.hasFLOWOFFLOADING) o.value('flow_offloading', _('Flow offloading'));
		if (features.hasFASTCLASSIFIER) o.value('fast_classifier', _('Fast classifier'));
		if (features.hasSHORTCUTFECM) o.value('shortcut_fe_cm', _('SFE connection manager'));
		if (features.hasMEDIATEKHNAT) o.value('mediatek_hnat', _('MediaTek HNAT'));

		o.default = 'disabled';

		const descMap = {
			'flow_offloading': _('Software based offloading for routing/NAT.'),
			'fast_classifier': _('Fast classifier connection manager for the shortcut forwarding engine.'),
			'shortcut_fe_cm': _('Simple connection manager for the shortcut forwarding engine.'),
			'mediatek_hnat': _('MediaTek\'s open source hardware offloading engine.'),
			'default': _('The offloading engine for routing/NAT.')
		};

		o.onchange = function(ev, section_id, value) {
			let desc = descMap[value] || descMap['default'];
			let el = this.getUIElement(section_id);
			if (el && el.node && el.node.parentNode) {
				let descEl = el.node.parentNode.querySelector('.cbi-value-description');
				if (descEl) descEl.innerHTML = desc;
			}
		};

		o = s.option(form.Flag, 'fastpath_fo_hw', _('Hardware flow offloading'),
			_('Requires hardware NAT support. Implemented at least for mt7621.'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'flow_offloading');

		o = s.option(form.Flag, 'fastpath_fc_br', _('Bridge Acceleration'),
			_('Enable bridge acceleration (may be functional conflict with bridge-mode VPN server).'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'fast_classifier');

		if (features.hasIPV6) {
			o = s.option(form.Flag, 'fastpath_fc_ipv6', _('IPv6 acceleration'),
				_('Enable IPv6 Acceleration.'));
			o.default = o.disabled;
			o.rmempty = false;
			o.depends('fastpath', 'fast_classifier');
		}

		o = s.option(form.Value, 'fastpath_mh_bind_rate', _('HNAT bind rate threshold (pps)'),
			_('The smaller the threshold, the easier it is for the connection to be accelerated.'));
		o.optional = true;
		o.datatype = 'range(1,30)';
		o.placeholder = 30;
		o.default = 30;
		o.depends('fastpath', 'mediatek_hnat');

		o = s.option(form.Flag, 'fastpath_mh_update_nfct', _('Enable HNAT counter update'),
		_('Update HNAT counter to nf_conntrack. May impact performance.'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'mediatek_hnat');

		o = s.option(form.ListValue, 'tcpcca', _('TCP CCA'),
			_('TCP congestion control algorithm.'));
		
		if (features.hasTCPCCA) {
			let algos = features.hasTCPCCA.split(' ').sort();
			for (let i = 0; i < algos.length; i++) {
				o.value(algos[i]);
			}
		}
		o.default = 'cubic';
		o.rmempty = false;

		return m.render();
	}
});
