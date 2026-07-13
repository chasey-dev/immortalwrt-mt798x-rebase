'use strict';
'require dom';
'require form';
'require poll';
'require rpc';
'require uci';
'require ui';
'require view';

const PWM_MAX = 255;

const callStatus = rpc.declare({
	object: 'luci.fancontrol',
	method: 'getStatus',
	params: [ 'hwmon' ],
	expect: { '': {} }
});

const callSetPwm = rpc.declare({
	object: 'luci.fancontrol',
	method: 'setPwm',
	params: [ 'pwm', 'hwmon' ],
	expect: { '': {} }
});

const callServiceList = rpc.declare({
	object: 'service',
	method: 'list',
	params: [ 'name' ],
	expect: { '': {} }
});

function getServiceStatus() {
	return L.resolveDefault(callServiceList('fancontrol'), {}).then(function(res) {
		let instances = res.fancontrol ? res.fancontrol.instances : null;

		for (let name in (instances || {}))
			if (instances[name].running)
				return true;

		return false;
	});
}

function getSettings() {
	return {
		enabled: uci.get('fancontrol', 'settings', 'enabled') === '1',
		mode: uci.get('fancontrol', 'settings', 'mode') || 'auto',
		hwmon: uci.get('fancontrol', 'settings', 'hwmon') || 'auto',
		temp_source: uci.get('fancontrol', 'settings', 'temp_source') || 'auto'
	};
}

function pwmToPercent(value) {
	let pwm = parseInt(value, 10);

	if (isNaN(pwm))
		return null;

	return Math.round(Math.max(0, Math.min(PWM_MAX, pwm)) * 100 / PWM_MAX);
}

function percentToPwm(value) {
	let percent = parseInt(value, 10);

	if (isNaN(percent))
		return null;

	return Math.round(Math.max(0, Math.min(100, percent)) * PWM_MAX / 100);
}

function formatTemperature(sensor) {
	if (!sensor || sensor.temp_mC == null)
		return _('Not reported');

	return _('%.1f °C').format(sensor.temp_mC / 1000);
}

function statusBadge(text, warning) {
	return E('span', { 'class': warning ? 'label warning' : 'label success' }, text);
}

function renderProgress(value) {
	let percent = pwmToPercent(value) || 0;

	return E('div', {
		'class': 'cbi-progressbar',
		'title': _('%d%% (%d / 255)').format(percent, value)
	}, E('div', { 'style': 'width:%d%%'.format(percent) }));
}

function statusRow(label, value, detail, index) {
	let content = [ value ];

	if (detail)
		content.push(E('div', { 'class': 'cbi-value-description' }, detail));

	return E('tr', { 'class': 'tr cbi-rowstyle-%d'.format(index % 2 + 1) }, [
		E('td', { 'class': 'td left' }, label),
		E('td', { 'class': 'td left' }, content)
	]);
}

function selectTemperature(temperatures, wanted) {
	if (!temperatures.length)
		return null;

	if (wanted && wanted !== 'auto') {
		for (let sensor of temperatures)
			if (sensor.path === wanted)
				return sensor;

		return null;
	}

	for (let sensor of temperatures)
		if (sensor.name === 'cpu-thermal')
			return sensor;

	return temperatures[0];
}

function renderStatus(status, running, settings) {
	status = status || {};

	let rows = [];
	let fan = status.fan || {};
	let temperatures = Array.isArray(status.temperatures) ? status.temperatures : [];
	let sensor = selectTemperature(temperatures,
		settings.mode === 'auto' ? settings.temp_source : 'auto');
	let ready = status.ok && fan.path;
	let serviceState, serviceDetail, temperatureValue, temperatureDetail;

	if (running) {
		serviceState = statusBadge(_('Running'));
		serviceDetail = settings.mode === 'auto' ?
			_('Temperature curve mode') : _('Fixed speed mode');
	}
	else if (settings.enabled) {
		serviceState = statusBadge(_('Not running'), true);
		serviceDetail = _('The service is enabled but is not running.');
	}
	else {
		serviceState = E('span', { 'class': 'label' }, _('Disabled'));
		serviceDetail = _('Enable fan control below to start the service.');
	}

	if (sensor) {
		temperatureValue = formatTemperature(sensor);
		temperatureDetail = E('span', { 'title': sensor.path || '' },
			sensor.name || _('Temperature sensor'));
	}
	else if (settings.mode === 'auto' && settings.temp_source !== 'auto') {
		temperatureValue = statusBadge(_('Not found'), true);
		temperatureDetail = settings.temp_source;
	}
	else {
		temperatureValue = _('Not reported');
	}

	rows.push(statusRow(_('Status'), serviceState, serviceDetail, rows.length));
	rows.push(statusRow(_('Fan device'),
		ready ? statusBadge(_('Detected')) : statusBadge(_('Not detected'), true),
		ready ? E('span', { 'title': fan.realpath || fan.path },
			fan.name || fan.path.split('/').pop()) : null,
		rows.length));
	rows.push(statusRow(settings.mode === 'auto' ? _('Control temperature') : _('Temperature'),
		temperatureValue, temperatureDetail, rows.length));
	rows.push(statusRow(_('Fan speed'),
		fan.rpm == null ? _('Not reported') : _('%d RPM').format(fan.rpm),
		null, rows.length));
	rows.push(statusRow(_('PWM output'),
		fan.pwm == null ? _('Not reported') : E('div', {}, [
			E('strong', {}, _('%d%%').format(pwmToPercent(fan.pwm))),
			renderProgress(fan.pwm)
		]),
		fan.pwm == null ? null : _('Raw PWM value: %d / 255').format(fan.pwm),
		rows.length));

	return [
		!ready ? E('p', { 'class': 'alert-message warning' },
			_('No compatible PWM fan was detected. Check the hardware settings and pwm-fan driver.')) : '',
		E('table', { 'class': 'table' }, rows)
	];
}

function optionValue(map, name, section_id, fallback) {
	let option = L.toArray(map.lookupOption(name, section_id))[0];
	let value = option ? option.formvalue(section_id) : null;

	if ((value == null || value === '') && option)
		value = option.cfgvalue(section_id);

	if ((value == null || value === '') && option)
		value = option.default;

	return (value == null || value === '') ? fallback : value;
}

function configurePwmSlider(option, defaultPwm) {
	option.min = 0;
	option.max = 100;
	option.step = 1;
	option.datatype = 'range(0,100)';
	option.default = String(pwmToPercent(defaultPwm));
	option.optional = true;
	option.rmempty = true;
	option.cfgvalue = function(section_id, setValue) {
		if (arguments.length === 2) {
			return form.RangeSliderValue.prototype.cfgvalue.call(this, section_id,
				setValue == null ? null : String(pwmToPercent(setValue)));
		}

		return form.RangeSliderValue.prototype.cfgvalue.call(this, section_id);
	};
	option.write = function(section_id, value) {
		return form.RangeSliderValue.prototype.write.call(this, section_id,
			String(percentToPwm(value)));
	};
}

function refreshStatus(status) {
	let settings = getSettings();

	return Promise.all([
		status != null ? status : L.resolveDefault(callStatus(settings.hwmon), {}),
		L.resolveDefault(getServiceStatus(), false)
	]).then(function(data) {
		let node = document.getElementById('fancontrol_status_content');

		if (node)
			dom.content(node, renderStatus(data[0], data[1], settings));

		return data[0];
	});
}

return view.extend({
	load: function() {
		return uci.load('fancontrol').then(function() {
			let settings = getSettings();

			return Promise.all([
				L.resolveDefault(callStatus(settings.hwmon), {}),
				L.resolveDefault(getServiceStatus(), false)
			]);
		});
	},

	render: function(data) {
		let m, s, o;
		let settings = getSettings();
		let initialStatus = data[0] || {};
		let fan = initialStatus.fan || {};
		let temperatures = Array.isArray(initialStatus.temperatures) ?
			initialStatus.temperatures : [];

		m = new form.Map('fancontrol', _('Fan Control'),
			_('Monitor the cooling system and control the PWM fan with a fixed speed or a temperature curve.'));

		s = m.section(form.TypedSection);
		s.anonymous = true;
		s.render = function() {
			poll.add(function() {
				return refreshStatus();
			});

			return E('div', { 'class': 'cbi-section' }, [
				E('h3', {}, _('Live status')),
				E('div', { 'class': 'cbi-section-node', 'id': 'fancontrol_status_content' },
					renderStatus(initialStatus, data[1], settings))
			]);
		};

		s = m.section(form.NamedSection, 'settings', 'fancontrol', _('Settings'));
		s.anonymous = true;
		s.addremove = false;
		s.tab('general', _('General'));
		s.tab('curve', _('Temperature curve'));
		s.tab('hardware', _('Hardware'));

		o = s.taboption('general', form.Flag, 'enabled', _('Enable fan control'));
		o.description = _('Run the fan control service after the configuration is applied.');
		o.default = o.disabled;
		o.rmempty = false;

		o = s.taboption('general', form.RichListValue, 'mode', _('Control mode'));
		o.value('auto', _('Temperature curve'),
			_('Adjust the fan continuously between two temperature and speed points.'));
		o.value('manual', _('Fixed speed'),
			_('Keep the fan at one constant output.'));
		o.widget = 'radio';
		o.orientation = 'horizontal';
		o.default = 'auto';
		o.rmempty = false;

		o = s.taboption('general', form.RangeSliderValue, 'manual_pwm', _('Fixed fan speed (%)'));
		o.description = _('Fan output used in fixed speed mode.');
		configurePwmSlider(o, 128);
		o.depends('mode', 'manual');

		o = s.taboption('general', form.Button, '_test_pwm', _('Test fan speed'));
		o.description = _('Apply the selected speed immediately without saving it or starting the service.');
		o.inputtitle = _('Apply test');
		o.inputstyle = 'action';
		o.depends('mode', 'manual');
		o.onclick = function(ev, section_id) {
			let percent = optionValue(this.map, 'manual_pwm', section_id, '50');
			let pwm = percentToPwm(percent);
			let hwmon = optionValue(this.map, 'hwmon', section_id, 'auto');
			let button = ev ? ev.currentTarget : null;

			if (button) {
				button.disabled = true;
				button.blur();
			}

			return callSetPwm(pwm, hwmon).then(function(res) {
				if (!res || !res.ok) {
					ui.addNotification(null, E('p', {},
						(res && res.error) || _('Failed to apply the test speed.')), 'error');
				}
				else {
					ui.addNotification(null, E('p', {},
						_('Fan output set to %d%%.').format(percent)), 'info');
				}

				return refreshStatus(res);
			}).catch(function(err) {
				ui.addNotification(null, E('p', {},
					_('Failed to apply the test speed: %s').format(err.message || err)), 'error');
			}).finally(function() {
				if (button)
					button.disabled = false;
			});
		};

		o = s.taboption('curve', form.RangeSliderValue, 'low_temp', _('Start temperature (°C)'));
		o.description = _('At or below this temperature, the fan uses the minimum speed.');
		o.min = 0;
		o.max = 125;
		o.step = 1;
		o.datatype = 'range(0,125)';
		o.default = '45';
		o.optional = true;
		o.rmempty = true;
		o.depends('mode', 'auto');

		o = s.taboption('curve', form.RangeSliderValue, 'min_pwm', _('Minimum fan speed (%)'));
		o.description = _('Fan output used at the start temperature.');
		configurePwmSlider(o, 80);
		o.depends('mode', 'auto');

		o = s.taboption('curve', form.RangeSliderValue, 'high_temp', _('Full-speed temperature (°C)'));
		o.description = _('At or above this temperature, the fan uses the maximum speed.');
		o.min = 0;
		o.max = 125;
		o.step = 1;
		o.datatype = 'range(0,125)';
		o.default = '75';
		o.optional = true;
		o.rmempty = true;
		o.depends('mode', 'auto');
		o.validate = function(section_id, value) {
			let lowTemp = optionValue(this.map, 'low_temp', section_id, '45');

			if (value == null || value === '')
				return true;

			return parseInt(value, 10) > parseInt(lowTemp, 10) ||
				_('Full-speed temperature must be higher than the start temperature.');
		};

		o = s.taboption('curve', form.RangeSliderValue, 'max_pwm', _('Maximum fan speed (%)'));
		o.description = _('Fan output used at the full-speed temperature.');
		configurePwmSlider(o, 255);
		o.depends('mode', 'auto');
		o.validate = function(section_id, value) {
			let minPwm = optionValue(this.map, 'min_pwm', section_id, '31');

			if (value == null || value === '')
				return true;

			return parseInt(value, 10) >= parseInt(minPwm, 10) ||
				_('Maximum fan speed cannot be lower than the minimum fan speed.');
		};

		o = s.taboption('curve', form.Value, 'interval', _('Update interval'));
		o.description = _('How often the temperature is checked and the fan output is updated.');
		o.value('1', '1 s');
		o.value('2', '2 s');
		o.value('5', '5 s');
		o.value('10', '10 s');
		o.value('30', '30 s');
		o.value('60', '60 s');
		o.datatype = 'range(1,3600)';
		o.default = '5';
		o.rmempty = false;
		o.depends('mode', 'auto');

		o = s.taboption('hardware', form.Value, 'hwmon', _('Fan device'));
		o.description = _('Use automatic detection unless more than one compatible PWM fan is present.');
		o.value('auto', _('Automatic detection'));
		if (fan.path) {
			o.value(fan.path, _('%s (%s)').format(
				fan.name || 'pwmfan', fan.path.split('/').pop()));
		}
		o.default = 'auto';
		o.rmempty = false;

		o = s.taboption('hardware', form.Value, 'temp_source', _('Temperature sensor'));
		o.description = _('Use automatic detection to prefer cpu-thermal, or select a detected sensor.');
		o.value('auto', _('Automatic detection'));
		for (let sensor of temperatures) {
			o.value(sensor.path, '%s — %s'.format(
				sensor.name || sensor.path.split('/').pop(), formatTemperature(sensor)));
		}
		o.default = 'auto';
		o.rmempty = false;
		o.depends('mode', 'auto');

		return m.render();
	}
});
