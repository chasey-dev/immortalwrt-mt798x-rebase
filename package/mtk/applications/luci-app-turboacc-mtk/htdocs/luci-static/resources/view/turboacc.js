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

let callFullConeCounters = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeCounters',
	expect: { '': {} }
});

let callFullConeFlows = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeFlows',
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

	return E('em', { 'style': 'color:green; font-weight:bold' }, stat.type);
}

function renderCounterTable(rows) {
	if (!rows || !rows.length)
		return E('em', {}, _('No fullcone rule is active in the running ruleset.'));

	let trs = [
		E('tr', { 'class': 'tr table-titles' }, [
			E('th', { 'class': 'th' }, _('Direction')),
			E('th', { 'class': 'th' }, _('Protocol')),
			E('th', { 'class': 'th' }, _('Packets')),
			E('th', { 'class': 'th' }, _('Bytes'))
		])
	];

	for (let i = 0; i < rows.length; i++) {
		let r = rows[i];
		trs.push(E('tr', { 'class': 'tr' }, [
			E('td', { 'class': 'td tdLeft' }, r.direction),
			E('td', { 'class': 'td tdLeft' }, r.proto),
			E('td', { 'class': 'td' }, String(r.packets)),
			E('td', { 'class': 'td' }, String.format('%1024.2mB', r.bytes))
		]));
	}

	return E('table', { 'class': 'table' }, trs);
}

function renderFlowTable(res) {
	let rows = (res && res.rows) || [];

	if (!rows.length)
		return E('div', {}, E('em', {}, _('No inbound flow produced by fullcone yet.')));

	let trs = [
		E('tr', { 'class': 'tr table-titles' }, [
			E('th', { 'class': 'th' }, _('Peer')),
			E('th', { 'class': 'th' }, _('LAN host')),
			E('th', { 'class': 'th' }, _('Protocol')),
			E('th', { 'class': 'th' }, _('Port')),
			E('th', { 'class': 'th' }, _('Packets')),
			E('th', { 'class': 'th' }, _('Bytes'))
		])
	];

	for (let i = 0; i < rows.length; i++) {
		let r = rows[i];

		trs.push(E('tr', { 'class': 'tr' }, [
			E('td', { 'class': 'td tdLeft' }, r.peer),
			E('td', { 'class': 'td tdLeft' }, r.host),
			E('td', { 'class': 'td tdLeft' }, r.proto),
			E('td', { 'class': 'td' }, String(r.port)),
			E('td', { 'class': 'td' }, String(r.packets)),
			E('td', { 'class': 'td' }, String.format('%1024.2mB', r.bytes))
		]));
	}

	return E('table', { 'class': 'table' }, trs);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('turboacc'),
			uci.load('firewall'),
			L.resolveDefault(callSystemFeatures(), {}),
			L.resolveDefault(callMTKPPEStat(), {})
		]).then(function([turboacc, firewall, features, ppe_stats]) {
			return { features: features, ppe_stats: ppe_stats };
		});
	},

	render: function(data) {
		let features = data.features;
		let ppe_stats = data.ppe_stats;

		/* only check ppe when mtk_hnat is enabled */
		let current_fastpath = uci.get('turboacc', 'config', 'fastpath');
		let has_ppe = (current_fastpath == 'mediatek_hnat') && ppe_stats && ppe_stats['PPE num'];

		let m, s, o;

		m = new form.Map('turboacc', _('TurboACC'),
			_('Open source flow offloading engine (fast path or hardware NAT).'));

		m.tabbed = true;

		/* the status card is rendered at the top of the settings tab */
		let buildStatusCard = function () {
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


			return acc_status;
		};

		s = m.section(form.NamedSection, 'config', 'turboacc', _('TurboACC settings'));
		s.hidetitle = true;

		/* status card on top of the engine settings. The node returned by
		 * renderContents() IS the section element and carries the data-tab /
		 * data-tab-title attributes the tab bar reads, so the card has to be
		 * inserted into it - wrapping it in a new element would lose them and
		 * leave the tab label empty. */
		s.renderContents = function() {
			return Promise.resolve(form.NamedSection.prototype.renderContents.apply(this, arguments))
				.then(function(sectionEl) {
					sectionEl.insertBefore(buildStatusCard(), sectionEl.firstChild);
					return sectionEl;
				});
		};

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


		/* --------------------------------------------------- fullcone NAT tab */
		s = m.section(form.TypedSection, '_fullcone', _('Fullcone NAT'));
		s.anonymous = true;
		s.addremove = false;
		s.hidetitle = true;
		s.cfgsections = function() { return ['_fullcone']; };
		s.description = _('Endpoint-independent mapping and filtering for masqueraded traffic. The kernel keeps a second hash table inside conntrack, keyed by the translated 3-tuple (protocol, source address, source port), so peers can reach a client from an address it never contacted.') +
			'<br /><br />' +
			_('These are master gates. With a gate off, no zone gets fullcone regardless of its own setting.');

		/* Hit counters are appended after the zone grid. Same rule as on the
		 * settings tab: renderContents() returns the section element itself and
		 * that element carries data-tab / data-tab-title, so the card has to be
		 * inserted into it - wrapping it in a new element would blank the label. */
		s.renderContents = function() {
			return Promise.resolve(form.TypedSection.prototype.renderContents.apply(this, arguments))
				.then(function(sectionEl) {
					sectionEl.appendChild(E('div', { 'id': 'fullcone_counters' },
						E('em', {}, _('Collecting data...'))));
					sectionEl.appendChild(E('div', { 'id': 'fullcone_flows' },
						E('em', {}, _('Collecting data...'))));
					return sectionEl;
				});
		};

		/* these options live in /etc/config/firewall */
		o = s.option(form.Flag, 'fullcone', _('Fullcone NAT (IPv4)'),
			_('Master gate for IPv4 fullcone.'));
		o.default = '0';
		o.rmempty = true;
		o.cfgvalue = function() { return uci.get('firewall', '@defaults[0]', 'fullcone'); };
		o.write = function(section_id, value) { uci.set('firewall', '@defaults[0]', 'fullcone', value); };
		o.remove = function() { uci.unset('firewall', '@defaults[0]', 'fullcone'); };

		o = s.option(form.Flag, 'fullcone6', _('Fullcone NAT (IPv6)'),
			_('Master gate for IPv6. Most IPv6 setups do not need fullcone.'));
		o.default = '0';
		o.rmempty = true;
		o.cfgvalue = function() { return uci.get('firewall', '@defaults[0]', 'fullcone6'); };
		o.write = function(section_id, value) { uci.set('firewall', '@defaults[0]', 'fullcone6', value); };
		o.remove = function() { uci.unset('firewall', '@defaults[0]', 'fullcone6'); };

		o = s.option(form.SectionValue, '__zones__', form.GridSection, 'zone',
			_('Per-zone control'),
			_('Fullcone only takes effect on zones that masquerade IPv4 traffic, normally just the wan zone.'));
		let gs = o.subsection;

		/* The zone sections live in /etc/config/firewall, not in 'turboacc'. */
		gs.uciconfig = 'firewall';

		/* The edit modal resolves its section with parent.data.get(parent.config,
		 * section_id) - parent.config being this map's config, 'turboacc'. The
		 * zone is not there, the lookup returns null and `sref['.name']` throws
		 * "Cannot read properties of null". Swap the config in only for that
		 * call, so the grid keeps its plain-text columns and its Edit button. */
		let orig_modal = gs.renderMoreOptionsModal;

		gs.renderMoreOptionsModal = function(section_id, ev) {
			let map = this.map, cfg = map.config;

			map.config = this.uciconfig || cfg;

			let restore = function(r) { map.config = cfg; return r; };

			return Promise.resolve(orig_modal.call(this, section_id, ev))
				.then(restore, function(e) { restore(); throw e; });
		};

		gs.anonymous = true;
		gs.addremove = false;
		gs.sortable = false;
		gs.cfgsections = function() {
			return uci.sections('firewall', 'zone').map(function(z) { return z['.name']; });
		};

		let zo;

		zo = gs.option(form.DummyValue, 'name', _('Zone'));
		zo.cfgvalue = function(section_id) { return uci.get('firewall', section_id, 'name') || section_id; };

		zo = gs.option(form.DummyValue, 'masq', _('IPv4 masquerading'));
		zo.cfgvalue = function(section_id) {
			return uci.get('firewall', section_id, 'masq') == '1' ? _('enabled') : _('disabled');
		};

		zo = gs.option(form.Flag, 'fullcone', _('Fullcone NAT'),
			_('Enable fullcone for this zone. Uncheck to keep plain masquerading here.'));
		zo.default = '1';
		zo.rmempty = false;
		zo.cfgvalue = function(section_id) { return uci.get('firewall', section_id, 'fullcone'); };
		zo.write = function(section_id, value) { uci.set('firewall', section_id, 'fullcone', value); };
		zo.remove = function(section_id) { uci.unset('firewall', section_id, 'fullcone'); };

		zo = gs.option(form.MultiValue, 'fullcone_proto', _('Protocols'),
			_('Restrict fullcone to the selected L4 protocols; every other protocol keeps plain masquerading. Leave empty to apply fullcone to all protocols.'));
		zo.value('udp', _('UDP'));
		zo.value('tcp', _('TCP'));
		zo.value('udplite', _('UDP-Lite'));
		zo.value('sctp', _('SCTP'));
		zo.value('dccp', _('DCCP'));
		zo.optional = true;
		zo.cfgvalue = function(section_id) { return uci.get('firewall', section_id, 'fullcone_proto'); };
		zo.write = function(section_id, value) { uci.set('firewall', section_id, 'fullcone_proto', value); };
		zo.remove = function(section_id) { uci.unset('firewall', section_id, 'fullcone_proto'); };

		return m.render().then(function(nodes) {
			/* register the status poll once, for the whole page */
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
			/* the counters refresh on their own, slower cadence: the backend
			 * shells out to nft, which is not worth doing at the status-card rate. */
			poll.add(async function() {
				let cel = document.getElementById('fullcone_counters');
				let fel = document.getElementById('fullcone_flows');

				if (!cel && !fel)
					return;

				if (cel) {
					let res = await L.resolveDefault(callFullConeCounters(), { rows: [] });
					L.dom.content(cel, renderCounterTable(res.rows));
				}

				if (fel) {
					let res = await L.resolveDefault(callFullConeFlows(), {});
					L.dom.content(fel, renderFlowTable(res));
				}
			}, 10);

			return nodes;
		});
	}
});
