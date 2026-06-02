--This file is created for beacon control

local mtkdat = require("mtkdat")
local nixio = require("nixio")

function set_mld_bcn_down()
	local uci = mtkdat.uci_load_wireless()
	local devs, l1parser = mtkdat.__get_l1dat()
	local tmp_dev

	for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
		if _devname then
			local profiles = mtkdat.search_dev_and_profile()
			local path = profiles[_devname]
			if not mtkdat.exist("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last") then
				os.execute("cp -f "..path.." "..mtkdat.__profile_previous_settings_path(path))
			end
		end

		if l1parser and devs then
			tmp_dev = devs.devname_ridx[_devname]
			if not tmp_dev then
				nixio.syslog("err", "mtwifi: tmp_dev ".._devname.." not found!")
				return
			end

			local devname2 = string.gsub(_devname, "%.", "_")
			local uci_vifs = mtkdat.get_uci_vifs_by_dev_name(uci, devname2)

			for _, uci_vif in pairs(uci_vifs) do
				if tostring(uci_vif.mldgroup) ~= '0' and tostring(uci_vif.mldgroup) < '17' then
					os.execute("mwctl "..uci_vif[".name"].." set no_bcn 1")
				end
			end
		end
	end
end

function set_mld_bcn_up()
	local uci = mtkdat.uci_load_wireless()
	local devs, l1parser = mtkdat.__get_l1dat()
	local tmp_dev

	for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
		if _devname then
			local profiles = mtkdat.search_dev_and_profile()
			local path = profiles[_devname]
			if not mtkdat.exist("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last") then
				os.execute("cp -f "..path.." "..mtkdat.__profile_previous_settings_path(path))
			end
		end

		if l1parser and devs then
			tmp_dev = devs.devname_ridx[_devname]
			if not tmp_dev then
				nixio.syslog("err", "mtwifi: tmp_dev ".._devname.." not found!")
				return
			end

			local devname2 = string.gsub(_devname, "%.", "_")
			local uci_vifs = mtkdat.get_uci_vifs_by_dev_name(uci, devname2)

			for _, uci_vif in pairs(uci_vifs) do
				if tostring(uci_vif.mldgroup) ~= '0' and tostring(uci_vif.mldgroup) < '17' then
					os.execute("mwctl "..uci_vif[".name"].." set no_bcn 0")
				end
			end
		end
	end
end
