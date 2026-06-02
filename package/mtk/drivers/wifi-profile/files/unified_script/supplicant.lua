#!/usr/bin/env lua

local nixio = require("nixio")
local var_path = "/var/run/"
local var_supplicant_path = var_path.."wpa_supplicant/"
local wpa_cli = "/usr/sbin/wpa_cli"

function supp_setup_vif(dev, iface)
	local file_name = var_supplicant_path.."wpa_supplicant-"..iface[".name"]..".conf"
	local file

	file = io.open(file_name, "w+")

	io.output(file)
	if iface.logger_syslog then
		io.write("logger_syslog=", iface.logger_syslog, "\n")
	end
	if iface.logger_syslog_level then
		io.write("logger_syslog_level=", iface.logger_syslog_level, "\n")
	end
	if iface.logger_stdout then
		io.write("logger_stdout=", iface.logger_stdout, "\n")
	end
	if iface.logger_stdout_level then
		io.write("logger_stdout_level=", iface.logger_stdout_level, "\n")
	end
	io.write("ctrl_interface="..var_supplicant_path.."\n")
	io.write("update_config=1\n\n")
	io.write("bss_expiration_scan_count=1\n\n")

	if iface.dpp_config_processing ~= nil then
		io.write("dpp_config_processing="..iface.dpp_config_processing.."\n")
	end

	if iface.wps_device_name ~= nil then
		io.write("device_name=", iface.wps_device_name, "\n")
	else
		io.write("device_name=Wireless station\n")
	end
	if iface.wps_device_type ~= nil then
		io.write("device_type=", iface.wps_device_type, "\n")
	else
		io.write("device_type=6-0050F204-1\n")
	end
	if iface.wps_manufacturer ~= nil then
		io.write("manufacturer=", iface.wps_manufacturer, "\n")
	else
		io.write("manufacturer=MediaTek Inc.\n")
	end
	io.write("model_name=MediaTek Wireless Access Point\n")
	io.write("model_number=MT7988\n")
	io.write("serial_number=12345678\n")
	io.write("config_methods=display virtual_push_button keypad physical_push_button\n")

	if iface.wps_cred_processing ~= nil then
		io.write("wps_cred_processing=", iface.wps_cred_processing, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("wps_cred_processing=1\n")
		end
	end

	if iface.wps_cred_add_sae ~= nil then
		io.write("wps_cred_add_sae=", iface.wps_cred_add_sae, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("wps_cred_add_sae=1\n")
		end
	end

	if iface.ignore_old_scan_res ~= nil then
		io.write("ignore_old_scan_res=", iface.ignore_old_scan_res, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("ignore_old_scan_res=1\n")
		end
	end

	if iface.pmf ~= nil then
		io.write("pmf=", iface.pmf, "\n")
	elseif dev.map_mode ~= nil and dev.map_mode ~= "0" and dev.band == "6G" then
		io.write("pmf=2\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("pmf=1\n")
		end
	end

	if iface.sae_pwe ~= nil then
		io.write("sae_pwe=", iface.sae_pwe, "\n")
	else
		io.write("sae_pwe=2\n")
	end

	if iface.sae_groups ~= nil then
		if type(iface.sae_groups) == "string" then
			io.write("sae_groups="..iface.sae_groups.."\n")
		elseif type(iface.sae_groups) == "table" then
			io.write("sae_groups=")
			for i, grp in pairs(iface.sae_groups) do
				io.write(grp.." ")
			end
			io.write("\n")
		end
	end

	if iface.ssid == nil or iface.ssid == "" then
		io.close()
		return
	end

	if iface.autoscan_interval == nil then
		io.write("autoscan=periodic:30\n")
	elseif iface.autoscan_interval ~= "0" then
		io.write("autoscan=periodic:", iface.autoscan_interval, "\n")
	end

	io.write("network={\n")
	io.write("\tssid=\""..iface.ssid.."\"\n")
	if iface.bssid ~= nil then
		io.write("\tbssid="..iface.bssid.."\n")
	end
	io.write("\tscan_ssid=1\n")


	if iface.proto == nil then
		iface.proto = {}
	end

	if iface.wpa_key_mgmt == nil then
		iface.wpa_key_mgmt = {}
	end

	if iface.encryption == "none" or
		iface.encryption == nil then
		table.insert(iface.wpa_key_mgmt, "NONE")
	elseif iface.encryption == "psk+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "psk2+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk2+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk2+ccmp" then
		if iface.pmf_sha256 == "1" and iface.ieee80211w == "1" then
			table.insert(iface.wpa_key_mgmt, "WPA-PSK")
			table.insert(iface.wpa_key_mgmt, "WPA-PSK-SHA256")
		elseif iface.pmf_sha256 == "1" and iface.ieee80211w == "2" then
			table.insert(iface.wpa_key_mgmt, "WPA-PSK-SHA256")
		else
			table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		end
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "psk2-mixed+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk2-mixed+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "psk2-mixed+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "psk-mixed+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "sae" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
--			table.insert(iface.pairwise, "gcmp")
--			table.insert(iface.pairwise, "ccmp256")
--			table.insert(iface.pairwise, "gcmp256")
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae+ccmp" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae+gcmp" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae+ccmp256" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae+gcmp256" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "bip-gmac-256"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae-mixed" then
		table.insert(iface.wpa_key_mgmt, "SAE")
		table.insert(iface.wpa_key_mgmt, "WPA-PSK")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
		iface.ieee80211w = "1"
	elseif iface.encryption == "sae-ext" then
		table.insert(iface.wpa_key_mgmt, "SAE-EXT-KEY")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
--			table.insert(iface.pairwise, "gcmp")
--			table.insert(iface.pairwise, "ccmp256")
			table.insert(iface.pairwise, "gcmp256")
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae-ext+ccmp" then
		table.insert(iface.wpa_key_mgmt, "SAE-EXT-KEY")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae-ext+gcmp" then
		table.insert(iface.wpa_key_mgmt, "SAE-EXT-KEY")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae-ext+ccmp256" then
		table.insert(iface.wpa_key_mgmt, "SAE-EXT-KEY")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "sae-ext+gcmp256" then
		table.insert(iface.wpa_key_mgmt, "SAE-EXT-KEY")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "bip-gmac-256"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "wpa+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "wpa+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "wpa+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "wpa2+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "wpa2+ccmp" then
		if iface.pmf_sha256 == "1" and iface.ieee80211w == "1" then
			table.insert(iface.wpa_key_mgmt, "WPA-EAP")
			table.insert(iface.wpa_key_mgmt, "WPA-EAP-SHA256")
		elseif iface.pmf_sha256 == "1" and iface.ieee80211w == "2" then
			table.insert(iface.wpa_key_mgmt, "WPA-EAP-SHA256")
		else
			table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		end
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "wpa2+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "wpa-mixed+tkip" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
		end
	elseif iface.encryption == "wpa-mixed+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
	elseif iface.encryption == "wpa-mixed+tkip+ccmp" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP")
		table.insert(iface.proto, "WPA")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "tkip")
			table.insert(iface.pairwise, "ccmp")
		end
        elseif iface.encryption == "wpa3" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP-SHA256")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "wpa3-192" then
		table.insert(iface.wpa_key_mgmt, "WPA-EAP-SUITE-B-192")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "bip-gmac-256"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "owe" then
		table.insert(iface.wpa_key_mgmt, "OWE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
--			table.insert(iface.pairwise, "gcmp")
--			table.insert(iface.pairwise, "ccmp256")
--			table.insert(iface.pairwise, "gcmp256")
		end
	elseif iface.encryption == "owe+ccmp" then
		table.insert(iface.wpa_key_mgmt, "OWE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "owe+gcmp" then
		table.insert(iface.wpa_key_mgmt, "OWE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "owe+ccmp256" then
		table.insert(iface.wpa_key_mgmt, "OWE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "ccmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "ccmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "owe+gcmp256" then
		table.insert(iface.wpa_key_mgmt, "OWE")
		table.insert(iface.proto, "RSN")
		if iface.pairwise == nil then
			iface.pairwise = {}
			table.insert(iface.pairwise, "gcmp256")
		end
		if iface.group_cipher == nil then
			iface.group_cipher = "gcmp256"
		end
		if iface.group_mgmt_cipher == nil then
			iface.group_mgmt_cipher = "aes-128-cmac"
		end
		iface.ieee80211w = "2"
	elseif iface.encryption == "dpp" then
		table.insert(iface.wpa_key_mgmt, "DPP")
--[[
	elseif iface.encryption == "wep" or
	       iface.encryption == "wep+open" then
		io.write("\tkey_mgmt=NONE\n")
		io.write("\tauth_alg=OPEN\n")
	elseif iface.encryption == "wep+shared" then
		io.write("\tkey_mgmt=NONE\n")
		io.write("\tauth_alg=SHARED\n")
	elseif iface.encryption == "wep+auto" then
		io.write("\tkey_mgmt=NONE\n")
]]
	end


	if iface.ieee80211r == "1" then
		if iface.wpa_key_mgmt ~= nil then
			for i, wpa_key_mgmt in pairs(iface.wpa_key_mgmt) do
				if wpa_key_mgmt == "WPA-PSK" or
				   wpa_key_mgmt == "WPA-PSK-SHA256" then
					table.insert(iface.wpa_key_mgmt, "FT-PSK")
				elseif wpa_key_mgmt == "WPA-EAP" then
					table.insert(iface.wpa_key_mgmt, "FT-EAP")
				elseif wpa_key_mgmt == "WPA-EAP-SUITE-B-192" then
					table.insert(iface.wpa_key_mgmt, "FT-EAP-SHA384")
				elseif wpa_key_mgmt == "SAE" then
					table.insert(iface.wpa_key_mgmt, "FT-SAE")
				elseif wpa_key_mgmt == "SAE-EXT-KEY" then
					table.insert(iface.wpa_key_mgmt, "FT-SAE-EXT-KEY")
				end
			end
		end
	end

	if iface.wpa_key_mgmt ~= nil and #iface.wpa_key_mgmt > 0 then
		io.write("\tkey_mgmt="..table.concat(iface.wpa_key_mgmt, " ").."\n")

	end

	if iface.proto ~= nil and #iface.proto > 0 then
		io.write("\tproto="..table.concat(iface.proto, " ").."\n")
	end

	local cipher_map = {tkip="TKIP", ccmp="CCMP", gcmp="GCMP", ccmp256="CCMP-256", gcmp256="GCMP-256"}

	if iface.pairwise ~= nil then
		local pairwise = {}

		if type(iface.pairwise) == "string" then
			table.insert(pairwise, cipher_map[iface.pairwise])
		elseif type(iface.pairwise) == "table" then
			for i = 1, #iface.pairwise do
				table.insert(pairwise, cipher_map[iface.pairwise[i]])
			end
		end
		if #pairwise > 0 then
			io.write("\tpairwise="..table.concat(pairwise, " ").."\n")
		end
	end

	if iface.group_cipher and cipher_map[iface.group_cipher]then
		io.write("\tgroup="..cipher_map[iface.group_cipher].."\n")
	end

	if iface.group_mgmt_cipher then
		if iface.group_mgmt_cipher == "aes-128-cmac" then
			io.write("\tgroup_mgmt=AES-128-CMAC\n")
		elseif iface.group_mgmt_cipher == "bip-gmac-256" then
			io.write("\tgroup_mgmt=BIP-GMAC-256\n")
		elseif iface.group_mgmt_cipher == "bip-gmac-128" then
			io.write("\tgroup_mgmt=BIP-GMAC-128\n")
		elseif iface.group_mgmt_cipher == "bip-cmac-256" then
			io.write("\tgroup_mgmt=BIP-CMAC-256\n")
		end
	end

	if iface.owe_only ~= nil then
		io.write("\towe_only=", iface.owe_only, "\n")
	end

	if iface.owe_group ~= nil then
		io.write("\towe_group=", iface.owe_group, "\n")
	end

	if iface.ieee80211w ~= nil and iface.encryption ~= "none" then
		if tonumber(iface.ieee80211w) >= 0 and tonumber(iface.ieee80211w) <= 2 then
			io.write("\tieee80211w=", iface.ieee80211w, "\n")
			if tonumber(iface.ieee80211w) >= 1 and tonumber(iface.ieee80211w) <= 2 then
				if iface.beacon_prot ~= nil and iface.beacon_prot == "1" then
					io.write("\tbeacon_prot=1\n")
				end
				if iface.ocv ~= nil and iface.ocv == "1" then
					io.write("\tocv=1\n")
				end
			end
		end
	end

--[[
	function prepare_key_wep(key)
		if key == nil or #key == 0 then
			return ""
		end

		local len = #key

		if (len == 10 or len == 26 or len == 32) and key == string.match(key, '%x+') then
			return key
		elseif (len == 5 or len == 13 or len == 16) then
			return "\""..key.."\""
		end

		return ""
	end

	if string.find(iface.encryption, "wep") ~= nil then
		for i = 0, 3 do
			key = iface['key'..tostring(i + 1)]
			if key and key ~= '' then
				io.write("\twep_key", tostring(i), "=", prepare_key_wep(key), "\n")
			end
		end
		if iface.key == nil then
			io.write("\twep_tx_keyidx=0\n")
		else
			local idx = tonumber(iface.key)
			idx = idx - 1
			io.write("\twep_tx_keyidx=", tostring(idx), "\n")
		end
	end
]]

	if string.find(iface.encryption, "wpa") ~= nil then
		if iface.identity ~= nil then
			io.write("\tidentity=\"", iface.identity, "\"\n")
		end

		if iface.client_cert ~= nil then
			io.write("\tclient_cert=\"", iface.client_cert, "\"\n")
		end

		if iface.ca_cert ~= nil then
			io.write("\tca_cert=\"", iface.ca_cert, "\"\n")
		end

		if iface.client_cert ~= nil then
			io.write("\tclient_cert=\"", iface.client_cert, "\"\n")
		end

		if iface.priv_key ~= nil then
			io.write("\tprivate_key=\"", iface.priv_key, "\"\n")
		end

		if iface.priv_key_pwd ~= nil then
			io.write("\tprivate_key_passwd=\"", iface.priv_key_pwd, "\"\n")
		end

		if iface.eap_type ~= nil then
			io.write("\teap=", string.upper(iface.eap_type), "\n")
			if iface.eap_type == "peap" or
			   iface.eap_type == "ttls" then
				if iface.phase1 ~= nil then
					io.write("\tphase1=\"", iface.phase1, "\"\n")
				end

				if iface.auth ~= nil then
					io.write("\tphase2=\"auth=", iface.auth, "\"\n")
				else
					io.write("\tphase2=\"auth=MSCHAPV2\"\n")
				end

				if iface.password ~= nil then
					io.write("\tpassword=\"", iface.password, "\"\n")
				end
			end
		end
	end

	if iface.key then
		local hex_key = false

		if #iface.key == 64 then
			local i, j = string.find(iface.key, "%x+")
			if i == 1 and j == 64 then
				hex_key = true
			end
		end

		if string.find(iface.encryption, "psk") ~= nil then
			if hex_key == true then
				io.write("\tpsk="..iface.key.."\n")
			elseif #iface.key >= 8 and #iface.key <= 63 then
				io.write("\tpsk=\""..iface.key.."\"\n")
			end
		elseif iface.encryption == "sae-mixed" then
			if #iface.key >= 8 and #iface.key <= 63 then
				io.write("\tpsk=\""..iface.key.."\"\n")
			else
				nixio.syslog("err", "invalid psk length for sae-mixed")
			end
		elseif string.find(iface.encryption, "sae") ~= nil then
			if iface.sae_password == nil then
				io.write("\tsae_password=\""..iface.key.."\"\n")
			end
		end
	end

	if iface.sae_password then
		io.write("\tsae_password=\""..iface.sae_password.."\"\n")
	end

	if string.find(iface.encryption, "dpp") ~= nil then
		if iface.dpp_connector then
			io.write("\tdpp_connector=\""..iface.dpp_connector.."\"\n")
		end
		if iface.dpp_netaccesskey then
			io.write("\tdpp_netaccesskey=\""..iface.dpp_netaccesskey.."\"\n")
		end
		if iface.dpp_csign then
			io.write("\tdpp_csign=\""..iface.dpp_csign.."\"\n")
		end
	end

        if iface.transition_disable ~= nil then
		io.write("\ttransition_disable="..tostring(iface.transition_disable).."\n")
        end

	io.write("}\n")

	io.close()
end

function supp_enable_vif(iface)
	local file_name = var_supplicant_path.."wpa_supplicant-"..iface[".name"]..".conf"
	local action_scan_pid = var_path.."/action-"..iface[".name"].."-scan.pid"

	if iface.network then
		os.execute("ubus -t 20 wait_for network.interface."..iface.network)
		os.execute(wpa_cli.." -p "..var_supplicant_path.." -i global interface_add "..iface[".name"].." "..file_name.." \"\" \"\" \"\" br-"..iface.network)
	else
		os.execute(wpa_cli.." -p "..var_supplicant_path.." -i global interface_add "..iface[".name"].." "..file_name.." \"\" \"\" \"\" \"\"")
	end
	os.execute("exec 1000>&- && "..wpa_cli.." -i "..iface[".name"].." -a /lib/wifi/supplicant_scan_action.sh -B -P "..action_scan_pid)
end

function supp_disable_vif(iface)
	local file_name = var_supplicant_path.."wpa_supplicant-"..iface..".conf"
	local action_scan_pid = var_path.."/action-"..iface.."-scan.pid"
	local scan_state = var_path.."scan_state-"..iface

	os.execute("[ -f "..action_scan_pid.." ] && kill -SIGTERM `cat "..action_scan_pid.."` 2>/dev/null")
	os.execute(wpa_cli.." -p "..var_supplicant_path.." -i global interface_remove "..iface)
	os.remove(action_scan_pid)
	os.remove(scan_state)
	os.remove(file_name)
end
