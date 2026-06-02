#!/usr/bin/env lua

require("uci")

local iface = arg[1]
local event = arg[2]
local wps_state
local ssid
local ssid2
local wpa
local wpa_key_mgmt = {}
local ieee80211w
local sae_pwe
local sae_require_mfp
local wpa_pairwise = {}
local wpa_passphrase
local wpa_psk
local auth_algs

local function parse_config(line)
	if line == nil or line == "" then
		return
	end

	local result = {};

	for match in (line.."="):gmatch("(.-)=") do
		table.insert(result, match);
	end

	if result[1] == "wps_state" then
		wps_state = result[2]
	elseif result[1] == "ssid2" then
		ssid2 = result[2]
	elseif result[1] == "ssid" then
		ssid = result[2]
	elseif result[1] == "wpa" then
		wpa = result[2]
	elseif result[1] == "wpa_key_mgmt" then
		for match2 in (result[2].." "):gmatch("(.-) ") do
			table.insert(wpa_key_mgmt, match2);
		end
		table.sort(wpa_key_mgmt)
	elseif result[1] == "ieee80211w" then
		ieee80211w = result[2]
	elseif result[1] == "sae_pwe" then
		sae_pwe = result[2]
	elseif result[1] == "sae_require_mfp" then
		sae_require_mfp = result[2]
	elseif result[1] == "wpa_pairwise" then
		for match2 in (result[2].." "):gmatch("(.-) ") do
			table.insert(wpa_pairwise, match2);
		end
		table.sort(wpa_pairwise)
	elseif result[1] == "wpa_passphrase" then
		wpa_passphrase = result[2]
	elseif result[1] == "wpa_psk" then
		wpa_psk = result[2]
	elseif result[1] == "auth_algs" then
		auth_algs = result[2]
	end
end

local function translate_config()
	local x = uci.cursor()

	if auth_algs == nil or
	   auth_algs ~= "1" then
		-- TODO log here
		return
	end

	if wpa == nil and
	   #wpa_key_mgmt == 0 and
	   #wpa_pairwise == 0 then
		x:set("wireless", iface, "encryption", "none")
		x:delete("wireless", iface, "key")
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk+tkip+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "psk+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk+tkip")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk2+tkip+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "psk2+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk2+tkip")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk-mixed+tkip+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "psk-mixed+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "psk-mixed+tkip")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae")  -- sae+ccmp+gcmp256
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae+gcmp256")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE-EXT" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae-ext")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae+gcmp256")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE-EXT" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae-ext")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE-EXT" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae-ext+ccmp")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "SAE-EXT" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "sae-ext+gcmp256")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "2")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 2 and
	   wpa_key_mgmt[1] == "SAE" and
	   wpa_key_mgmt[2] == "WPA-PSK" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" and
	   ieee80211w == "1" then
		x:set("wireless", iface, "encryption", "sae-mixed")
		if wpa_passphrase then
			x:set("wireless", iface, "key", wpa_passphrase);
		elseif wpa_psk then
			x:set("wireless", iface, "key", wpa_psk);
		end
		x:set("wireless", iface, "ieee80211w", "1")
		if sae_pwe then
			x:set("wireless", iface, "sae_pwe", sae_pwe)
		else
			x:delete("wireless", iface, "sae_pwe")
		end
		if sae_require_mfp then
			x:set("wireless", iface, "sae_require_mfp", sae_require_mfp)
		else
			x:delete("wireless", iface, "sae_require_mfp")
		end
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa+tkip+ccmp")
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "wpa+ccmp")
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "1" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa+tkip")
		x:delete("wireless", iface, "ieee80211w")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa2+tkip+ccmp")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "wpa2+ccmp")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa2+tkip")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 2 and
	   wpa_pairwise[1] == "CCMP" and
	   wpa_pairwise[2] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa-mixed+tkip+ccmp")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" then
		x:set("wireless", iface, "encryption", "wpa-mixed+ccmp")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "3" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "TKIP" then
		x:set("wireless", iface, "encryption", "wpa-mixed+tkip")
		if ieee80211w then
			x:set("wireless", iface, "ieee80211w", ieee80211w)
		else
			x:set("wireless", iface, "ieee80211w", "0")
		end
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP-SHA256" and
	   (wpa_pairwise[0] == "CCMP" or
	    wpa_pairwise[1] == "CCMP") and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "wpa3")
		x:set("wireless", iface, "ieee80211w", "2")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 1 and
	   wpa_key_mgmt[1] == "WPA-EAP-SUITE-B-192" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "GCMP-256" and
	   ieee80211w == "2" then
		x:set("wireless", iface, "encryption", "wpa3-192")
		x:set("wireless", iface, "ieee80211w", "2")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	elseif wpa == "2" and
	   #wpa_key_mgmt == 2 and
	   wpa_key_mgmt[1] == "WPA-EAP" and
	   wpa_key_mgmt[2] == "WPA-EAP-SUITE-B-192" and
	   #wpa_pairwise == 1 and
	   wpa_pairwise[1] == "CCMP" and
	   ieee80211w == "1" then
		x:set("wireless", iface, "encryption", "wpa3-mixed")
		x:set("wireless", iface, "ieee80211w", "1")
		x:delete("wireless", iface, "sae_pwe")
		x:delete("wireless", iface, "sae_require_mfp")
	end
	x:set("wireless", iface, "ssid", ssid)
	x:set("wireless", iface, "wps_state", "2")
	x:commit("wireless")
end

-- Check the event
if event ~= "WPS-NEW-AP-SETTINGS" then
	return
end

-- Wait 1s for config write done
os.execute("sleep 1")


local fp = io.open("/var/run/hostapd/hostapd-"..iface..".conf", "r")
if fp == nil then
	return
end

local start_parse = 0

for line in fp:lines() do
	if line == "# WPS configuration - START" then
		start_parse = 1
	elseif line == "# WPS configuration - END" then
		start_parse = 0
		break
	end

	if start_parse == 1 then
		parse_config(line)
	end
end

fp:close()

translate_config()

