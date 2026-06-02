#!/usr/bin/env lua

local nixio = require("nixio")
local var_path = "/var/run/"
local var_hostapd_path = var_path.."hostapd/"
local hostapd_cli = "/usr/sbin/hostapd_cli"
local encryption_table = {
	-- none
	{
		["key"] = {
			["name"] = "none"
		},
		["auth_algs"] = "1"
	},

	-- psk+tkip
	{
		["key"] = {
			["name"] = "psk+tkip"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["ieee80211w"] = "0",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"tkip"}
	},

	-- psk+ccmp
	{
		["key"] = {
			["name"] = "psk+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["ieee80211w"] = "0",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp"}
	},

	-- psk+tkip+ccmp
	{
		["key"] = {
			["name"] = "psk+tkip+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["ieee80211w"] = "0",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- psk-mixed+tkip
	{
		["key"] = {
			["name"] = "psk-mixed+tkip",
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["ieee80211w"] = "0",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"tkip"}
	},

	-- psk-mixed+ccmp
	{
		["key"] = {
			["name"] = "psk-mixed+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp"}
	},

	-- psk-mixed+tkip+ccmp
	{
		["key"] = {
			["name"] = "psk-mixed+tkip+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- psk2, ieee80211w=1, pmf_sha256=1
	{
		["key"] = {
			["name"] = "psk2",
			["pmf_sha256"] = "1",
			["ieee80211w"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["pmf_sha256"] = "1",
		["ieee80211w"] = "1",
		["wpa_key_mgmt"] = {"WPA-PSK", "WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2, ieee80211w=2
	{
		["key"] = {
			["name"] = "psk2",
			["ieee80211w"] = "2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["ieee80211w"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2
	{
		["key"] = {
			["name"] = "psk2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- psk2+tkip
	{
		["key"] = {
			["name"] = "psk2+tkip"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"tkip"},
		["ieee80211w"] = "0"
	},

	-- psk2+ccmp, pmf_sha256=1, ieee80211w=1
	{
		["key"] = {
			["name"] = "psk2+ccmp",
			["pmf_sha256"] = "1",
			["ieee80211w"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["pmf_sha256"] = "1",
		["ieee80211w"] = "1",
		["wpa_key_mgmt"] = {"WPA-PSK", "WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2+ccmp, ieee80211w=2
	{
		["key"] = {
			["name"] = "psk2+ccmp",
			["ieee80211w"] = "2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["ieee80211w"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2+ccmp
	{
		["key"] = {
			["name"] = "psk2+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2+tkip+ccmp
	{
		["key"] = {
			["name"] = "psk2+tkip+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- sae, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae, rsne = 0, rsno = 1, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae",
			["rsne"] = "0",
			["rsno"] = "1",
			["rsno2"] = "0"
		},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_pairwise"] = {"ccmp"},
		["rsn_override_mfp"] = "2"
	},

	-- sae, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae
	{
		["key"] = {
			["name"] = "sae"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae+ccmp, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+ccmp",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1"
	},

	-- sae+ccmp, rsne = 0, rsno = 1, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+ccmp",
			["rsne"] = "0",
			["rsno"] = "1",
			["rsno2"] = "0"
		},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_pairwise"] = {"ccmp"},
		["rsn_override_mfp"] = "2"
	},

	-- sae+ccmp, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae+ccmp",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"ccmp"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+ccmp
	{
		["key"] = {
			["name"] = "sae+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae+gcmp, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+gcmp",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"gcmp"},
		["group_cipher"] = "gcmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+gcmp, rsne = 0, rsno = 1, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+gcmp",
			["rsne"] = "0",
			["rsno"] = "1",
			["rsno2"] = "0"
		},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_pairwise"] = {"gcmp"},
		["rsn_override_mfp"] = "2"
	},

	-- sae+gcmp, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae+gcmp",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"gcmp"},
		["rsn_override_pairwise_2"] = {"gcmp"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["group_cipher"] = "gcmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+gcmp
	{
		["key"] = {
			["name"] = "sae+gcmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"gcmp"},
		["group_cipher"] = "gcmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae+ccmp256, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+ccmp256",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["pairwise"] = {"ccmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["group_cipher"] = "ccmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+ccmp256, rsne = 0, rsno = 1, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+ccmp256",
			["rsne"] = "0",
			["rsno"] = "1",
			["rsno2"] = "0"
		},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_pairwise"] = {"gcmp"},
		["rsn_override_mfp"] = "2"
	},

	-- sae+ccmp256, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae+ccmp256",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp256"},
		["rsn_override_pairwise_2"] = {"ccmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["group_cipher"] = "ccmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+ccmp256
	{
		["key"] = {
			["name"] = "sae+ccmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp256"},
		["group_cipher"] = "ccmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae+gcmp256, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+gcmp256",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["pairwise"] = {"gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "bip-gmac-256",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},


	-- sae+gcmp256, rsne = 0, rsno = 1, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae+gcmp256",
			["rsne"] = "0",
			["rsno"] = "1",
			["rsno2"] = "0"
		},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_pairwise"] = {"gcmp256"},
		["rsn_override_mfp"] = "2"
	},

	-- sae+gcmp256, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae+gcmp256",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"gcmp256"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "bip-gmac-256",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae+gcmp256
	{
		["key"] = {
			["name"] = "sae+gcmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"gcmp256"},
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "bip-gmac-256",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae-mixed, pmf_sha256 = 1, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae-mixed",
			["pmf_sha256"] = "1",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "WPA-PSK", "WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-mixed, rsne = 1, rsno = 0, rsno2 = 0
	{
		["key"] = {
			["name"] = "sae-mixed",
			["rsne"] = "1",
			["rsno"] = "0",
			["rsno2"] = "0"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "WPA-PSK"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-mixed, pmf_sha256 = 1, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-mixed",
			["pmf_sha256"] = "1",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "WPA-PSK", "WPA-PSK-SHA256"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-mixed, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-mixed",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "WPA-PSK"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-mixed, pmf_sha256 = 1
	{
		["key"] = {
			["name"] = "sae-mixed",
			["pmf_sha256"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY", "WPA-PSK", "WPA-PSK-SHA256"},
		["pairwise"] = {"ccmp", "gcmp256"},
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-mixed
	{
		["key"] = {
			["name"] = "sae-mixed"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY", "WPA-PSK"},
		["pairwise"] = {"ccmp", "gcmp256"},
		["wps_cred_add_sae"] = "1",
		["sae_require_mfp"] = "1",
		["ieee80211w"] = "1",
		["_force_ieee80211w"] = "1",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-compat, 6g = 0, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-compat",
			["6g"] = "0",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK"},
		["rsn_override_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "0",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp"] = "2",
		["rsn_override_mfp_2"] = "2",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-compat, 6g = 1, rsne = 1, rsno = 1, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-compat",
			["6g"] = "1",
			["rsne"] = "1",
			["rsno"] = "1",
			["rsno2"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE"},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["pairwise"] = {"ccmp"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1",
		["rsn_override_mfp_2"] = "2",
		["beacon_prot"] = "1",
		["sae_groups"] = {"19", "20", "21"}
	},

	-- sae-ext, rsne = 0, rsno = 0, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-ext",
			["rsne"] = "0",
			["rsno"] = "0",
			["rsno2"] = "1"
		},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["rsn_override_mfp_2"] = "2"
	},

	-- sae-ext
	{
		["key"] = {
			["name"] = "sae-ext"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"},
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae-ext+ccmp, rsne = 0, rsno = 0, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-ext+ccmp",
			["rsne"] = "0",
			["rsno"] = "0",
			["rsno2"] = "1"
		},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["rsn_override_pairwise_2"] = {"ccmp"},
		["rsn_override_mfp_2"] = "2"
	},

	-- sae-ext+ccmp
	{
		["key"] = {
			["name"] = "sae-ext+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp"},
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae-ext+gcmp, rsne = 0, rsno = 0, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-ext+gcmp",
			["rsne"] = "0",
			["rsno"] = "0",
			["rsno2"] = "1"
		},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["rsn_override_pairwise_2"] = {"gcmp"},
		["rsn_override_mfp_2"] = "2"
	},

	-- sae-ext+gcmp
	{
		["key"] = {
			["name"] = "sae-ext+gcmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"gcmp"},
		["group_cipher"] = "gcmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae-ext+ccmp256, rsne = 0, rsno = 0, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-ext+ccmp256",
			["rsne"] = "0",
			["rsno"] = "0",
			["rsno2"] = "1"
		},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["rsn_override_pairwise_2"] = {"ccmp256"},
		["rsn_override_mfp_2"] = "2"
	},

	-- sae-ext+ccmp256
	{
		["key"] = {
			["name"] = "sae-ext+ccmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"ccmp256"},
		["group_cipher"] = "ccmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- sae-ext+gcmp256, rsne = 0, rsno = 0, rsno2 = 1
	{
		["key"] = {
			["name"] = "sae-ext+gcmp256",
			["rsne"] = "0",
			["rsno"] = "0",
			["rsno2"] = "1"
		},
		["rsn_override_key_mgmt_2"] = {"SAE-EXT-KEY"},
		["rsn_override_pairwise_2"] = {"gcmp256"},
		["rsn_override_mfp_2"] = "2"
	},

	-- sae-ext+gcmp256
	{
		["key"] = {
			["name"] = "sae-ext+gcmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE-EXT-KEY"},
		["pairwise"] = {"gcmp256"},
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "bip-gmac-256",
		["ieee80211w"] = "2",
		["_force_ieee80211w"] = "1"
	},

	-- owe
	{
		["key"] = {
			["name"] = "owe"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"OWE"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2"
	},

	-- owe+ccmp
	{
		["key"] = {
			["name"] = "owe+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"OWE"},
		["pairwise"] = {"ccmp"},
		["group_cipher"] = "ccmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2"
	},

	-- owe+gcmp
	{
		["key"] = {
			["name"] = "owe+gcmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"OWE"},
		["pairwise"] = {"gcmp"},
		["group_cipher"] = "gcmp",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2"
	},

	-- owe+ccmp256
	{
		["key"] = {
			["name"] = "owe+ccmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"OWE"},
		["pairwise"] = {"ccmp256"},
		["group_cipher"] = "ccmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2"
	},

	-- owe+gcmp256
	{
		["key"] = {
			["name"] = "owe+gcmp256"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"OWE"},
		["pairwise"] = {"gcmp256"},
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "aes-128-cmac",
		["ieee80211w"] = "2"
	},

	-- wpa+tkip
	{
		["key"] = {
			["name"] = "wpa+tkip"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"tkip"}
	},

	-- wpa+ccmp
	{
		["key"] = {
			["name"] = "wpa+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"ccmp"}
	},

	-- wpa+tkip+ccmp
	{
		["key"] = {
			["name"] = "wpa+tkip+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "1",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- wpa2, ieee80211w = 2
	{
		["key"] = {
			["name"] = "wpa2",
			["ieee80211w"] = "2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2"
	},

	-- wpa2, ieee80211w = 1, pmf_sha256 = 1
	{
		["key"] = {
			["name"] = "wpa2",
			["ieee80211w"] = "1",
			["pmf_sha256"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP", "WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1",
		["pmf_sha256"] = "1"
	},

	-- wpa2
	{
		["key"] = {
			["name"] = "wpa2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- wpa2+tkip
	{
		["key"] = {
			["name"] = "wpa2+tkip"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"tkip"}
	},

	-- wpa2+ccmp, ieee80211w = 2
	{
		["key"] = {
			["name"] = "wpa2+ccmp",
			["ieee80211w"] = "2"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2"
	},

	-- wpa2+ccmp, ieee80211w = 1, pmf_sha256 = 1
	{
		["key"] = {
			["name"] = "wpa2+ccmp",
			["ieee80211w"] = "1",
			["pmf_sha256"] = "1"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP", "WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1",
		["pmf_sha256"] = "1"
	},

	-- wpa2+ccmp
	{
		["key"] = {
			["name"] = "wpa2+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"ccmp"}
	},

        -- wpa2+tkip+ccmp
        {
                ["key"] = {
                        ["name"] = "wpa2+tkip+ccmp"
                },
                ["auth_algs"] = "1",
                ["wpa"] = "2",
                ["wpa_key_mgmt"] = {"WPA-EAP"},
                ["pairwise"] = {"tkip", "ccmp"}
        },

	-- wpa-mixed+tkip
	{
		["key"] = {
			["name"] = "wpa-mixed+tkip"
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"tkip"}
	},

	-- wpa-mixed+ccmp
	{
		["key"] = {
			["name"] = "wpa-mixed+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["wpa_key_mgmt"] = {"WPA-EAP", "WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1"
	},

	-- wpa-mixed+tkip+ccmp
	{
		["key"] = {
			["name"] = "wpa-mixed+tkip+ccmp"
		},
		["auth_algs"] = "1",
		["wpa"] = "3",
		["wpa_key_mgmt"] = {"WPA-EAP"},
		["pairwise"] = {"ccmp", "tkip"}
	},

	-- wpa3
	{
		["key"] = {
			["name"] = "wpa3"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "2"
	},

	-- wpa3-192
	{
		["key"] = {
			["name"] = "wpa3-192"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP-SUITE-B-192"},
		["pairwise"] = {"gcmp256"},
		["group_cipher"] = "gcmp256",
		["group_mgmt_cipher"] = "bip-gmac-256",
		["ieee80211w"] = "2"
	},

	-- wpa3-mixed
	{
		["key"] = {
			["name"] = "wpa3-mixed"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-EAP", "WPA-EAP-SHA256"},
		["pairwise"] = {"ccmp"},
		["ieee80211w"] = "1"
	},

	-- dpp
	{
		["key"] = {
			["name"] = "dpp"
		},
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"DPP"},
		["pairwise"] = {"ccmp"}
	},

	-- sae-dpp
	{
		["key"] = {
			["name"] = "sae-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "DPP"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2-dpp
	{
		["key"] = {
			["name"] = "psk2-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK", "DPP"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2-sae-dpp
	{
		["key"] = {
			["name"] = "psk2-sae-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK", "DPP", "SAE"},
		["pairwise"] = {"ccmp"}
	},

	-- psk2-sae-sae-ext-dpp
	{
		["key"] = {
			["name"] = "psk2-sae-sae-ext-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK", "DPP", "SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"}
	},

	-- sae-sae-ext-dpp
	{
		["key"] = {
			["name"] = "sae-sae-ext-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"DPP", "SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"}
	},

	-- psk2-sae-ext-dpp
	{
		["key"] = {
			["name"] = "psk2-sae-ext-dpp"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK", "DPP", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"}
	},

	-- sae-sae-ext
	{
		["key"] = {
			["name"] = "sae-sae-ext"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"}
	},

	-- psk2-sae-sae-ext
	{
		["key"] = {
			["name"] = "psk2-sae-sae-ext"
		},
		["auth_algs"] = "1",
		["wpa"] = "2",
		["wpa_key_mgmt"] = {"WPA-PSK", "SAE", "SAE-EXT-KEY"},
		["pairwise"] = {"ccmp", "gcmp256"}
	}
}

function find_encryption(name, ieee80211w, pmf_sha256, sixg, rsne, rsno, rsno2)
	if name == nil or name == "" then
		name = "none"
	end

	for i, encryption in ipairs(encryption_table) do
		if (encryption.key.name == name) and
		   (encryption.key.pmf_sha256 == nil or (encryption.key.pmf_sha256 and encryption.key.pmf_sha256 == pmf_sha256)) and
		   (encryption.key.ieee80211w == nil or (encryption.key.ieee80211w and encryption.key.ieee80211w == ieee80211w)) and
		   (encryption.key["6g"] == nil or (encryption.key["6g"] and encryption.key["6g"] == sixg)) and
		   (encryption.key.rsne == nil or (encryption.key.rsne and encryption.key.rsne == rsne)) and
		   (encryption.key.rsno == nil or (encryption.key.rsno and encryption.key.rsno == rsno)) and
		   (encryption.key.rsno2 == nil or (encryption.key.rsno2 and encryption.key.rsno2 == rsno2)) then
			return encryption
		end
	end

	return nil
end

function hostapd_setup_vif(dev, iface)
	local file_name = var_hostapd_path.."hostapd-"..iface[".name"]..".conf"
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

	io.write("interface=", iface[".name"], "\n")
	io.write("ssid=", iface.ssid, "\n")

--	if dev.country ~= nil then
--		io.write("country_code=", dev.country, "\n")
--	end

--	if dev.country_ie ~= nil then
--		io.write("ieee80211d=", dev.country_ie, "\n")
--	end

	io.write("bridge=", "br-", iface.network, "\n")  -- need to have network to bridge translate function
	local acs = false
	if string.lower(dev.channel) == "auto" or
	   dev.channel == nil or
	   dev.channel == "0" then
		io.write("channel=0\n")
		acs = true
	else
		io.write("channel=", dev.channel, "\n")
	end



	io.write("driver=nl80211\n")

	if dev.band == "2.4G" then
		if acs == true then
			io.write("hw_mode=any\n")
		else
			io.write("hw_mode=g\n")
		end
		io.write("preamble=1\n")
		io.write("ieee80211n=1\n")
		io.write("ieee80211ac=1\n")
		io.write("ieee80211ax=1\n")
		io.write("ieee80211be=1\n")
	elseif dev.band == "5G" then
		if acs == true then
			io.write("hw_mode=any\n")
		else
			io.write("hw_mode=a\n")
		end
		io.write("ieee80211n=1\n")
		io.write("ieee80211ac=1\n")
		io.write("ieee80211ax=1\n")
		io.write("ieee80211be=1\n")
	elseif dev.band == "6G" then
		if acs == true then
			io.write("hw_mode=any\n")
		else
			io.write("hw_mode=a\n")
		end
		io.write("ieee80211ax=1\n")
		io.write("ieee80211be=1\n")
		io.write("he_6ghz_max_mpdu=0\n")
		io.write("he_6ghz_max_ampdu_len_exp=0\n")
		io.write("he_6ghz_rx_ant_pat=0\n")
		io.write("he_6ghz_tx_ant_pat=0\n")
		io.write("op_class=131\n")
	end
	io.write("noscan=1\n")

	if iface.beacon_int ~= nil then
		if tonumber(iface.beacon_int) >= 15 and tonumber(iface.beacon_int) <= 65535 then
			io.write("beacon_int=", iface.beacon_int, "\n")
		end
	elseif dev.beacon_int ~= nil then
			if tonumber(dev.beacon_int) >= 15 and tonumber(dev.beacon_int) <= 65535 then
		io.write("beacon_int=", dev.beacon_int, "\n")
		end
	end

	if dev.band == "6G" then
		iface.ieee80211w = "2"
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			iface.ieee80211w = "1"
		end
	end


	if iface.dtim_period ~= nil then
		if tonumber(iface.dtim_period) >= 1 and tonumber(iface.dtim_period) <= 255 then
			io.write("dtim_period=", iface.dtim_period, "\n")
		end
	else
		io.write("dtim_period=1\n")
	end

	if iface.hidden ~= nil then
		if iface.hidden == "0" then
			io.write("ignore_broadcast_ssid=0\n")
		elseif iface.hidden == "2" then
			io.write("ignore_broadcast_ssid=2\n")
		else
			io.write("ignore_broadcast_ssid=1\n")
		end
	else
		io.write("ignore_broadcast_ssid=0\n")
	end

	if iface.dot11vmbssid ~= nil then
		io.write("dot11vmbssid=", iface.dot11vmbssid, "\n")
	end

	io.write("macaddr_acl=0\n")

	if iface.encryption == nil then
		iface.encryption = "none"
	end

	if iface.key == nil then
		iface.key = ""
	end

	if iface.mrsno_enable == nil then
		iface.mrsno_enable = "0"
	end

	if dev.band == "6G" then
		iface["6g"] = "1"
	else
		iface["6g"] = "0"
	end

	local encryption = nil
	if iface.mrsno_enable == "0" then
		encryption = find_encryption(iface.encryption, iface.ieee80211w, iface.pmf_sha256, iface["6g"], "0", "0", "0")
	else
		if iface.encryption_override == nil and iface.encryption_override_2 == nil then
			encryption = find_encryption(iface.encryption, iface.ieee80211w, iface.pmf_sha256, iface["6g"], "1", "1", "1")
		else
			encryption = find_encryption(iface.encryption, iface.ieee80211w, iface.pmf_sha256, iface["6g"], "1", "0", "0")
		end
	end
	if encryption == nil then
		nixio.syslog("err", "not supported encryption")
		return
	end

	for key, value in pairs(encryption) do
		if key ~= "key" and string.sub(key, 1, 7) ~= "_force_" then
			if iface[key] == nil or encryption["_force_"..key] == "1" then
				iface[key] = value
			end
		end
	end

	if iface.auth_algs ~= nil then
		io.write("auth_algs="..iface.auth_algs.."\n")
	end

	if iface.wpa ~= nil then
		io.write("wpa="..iface.wpa.."\n")
	end


	if iface.mrsno_enable  == "1" and iface.encryption_override then
		local encryption_override = find_encryption(iface.encryption_override, iface.ieee80211w_override, iface.pmf_sha256, iface["6g"], "0", "1", "0")
		if encryption_override then
			for key, value in pairs(encryption_override) do
				if key ~= "key" then
					if iface[key] == nil then
						iface[key] = value
					end
				end
			end
		end
	end

	if iface.mrsno_enable  == "1" and iface.encryption_override_2 then
		local encryption_override_2 = find_encryption(iface.encryption_override_2, iface.ieee80211w_override_2, iface.pmf_sha256, iface["6g"], "0", "0", "1")
		if encryption_override_2 then
			for key, value in pairs(encryption_override_2) do
				if key ~= "key" then
					if iface[key] == nil then
						iface[key] = value
					end
				end
			end
		end
	end

	if iface.ieee80211r == "1" then
		if iface.wpa_key_mgmt then
			for i, wpa_key_mgmt in pairs(iface.wpa_key_mgmt) do
				if wpa_key_mgmt == "WPA-PSK" or
				   wpa_key_mgmt == "WPA-PSK-SHA256" then
					table.insert(iface.wpa_key_mgmt, "FT-PSK")
					if iface.ft_psk_generate_local == nil then
						iface.ft_psk_generate_local = "1"
					end
				elseif wpa_key_mgmt == "WPA-EAP" or
					wpa_key_mgmt == "WPA-EAP-SHA256" then
					table.insert(iface.wpa_key_mgmt, "FT-EAP")
				elseif wpa_key_mgmt == "WPA-EAP-SUITE-B-192" then
					table.insert(iface.wpa_key_mgmt, "FT-EAP-SHA384")
				elseif wpa_key_mgmt == "SAE" then
					table.insert(iface.wpa_key_mgmt, "FT-SAE")
					if iface.ft_psk_generate_local == nil then
						iface.ft_psk_generate_local = "1"
					end
				elseif wpa_key_mgmt == "SAE-EXT-KEY" then
					table.insert(iface.wpa_key_mgmt, "FT-SAE-EXT-KEY")
					if iface.ft_psk_generate_local == nil then
						iface.ft_psk_generate_local = "1"
					end
				end
			end
		end

		if iface.rsn_override_key_mgmt then
			for i, wpa_key_mgmt in pairs(iface.rsn_override_key_mgmt) do
				if wpa_key_mgmt == "WPA-PSK" or
				   wpa_key_mgmt == "WPA-PSK-SHA256" then
					table.insert(iface.rsn_override_key_mgmt, "FT-PSK")
				elseif wpa_key_mgmt == "WPA-EAP" or
					wpa_key_mgmt == "WPA-EAP-SHA256" then
					table.insert(iface.rsn_override_key_mgmt, "FT-EAP")
				elseif wpa_key_mgmt == "WPA-EAP-SUITE-B-192" then
					table.insert(iface.rsn_override_key_mgmt, "FT-EAP-SHA384")
				elseif wpa_key_mgmt == "SAE" then
					table.insert(iface.rsn_override_key_mgmt, "FT-SAE")
				elseif wpa_key_mgmt == "SAE-EXT-KEY" then
					table.insert(iface.rsn_override_key_mgmt, "FT-SAE-EXT-KEY")
				end
			end
		end

		if iface.rsn_override_key_mgmt_2 then
			for i, wpa_key_mgmt in pairs(iface.rsn_override_key_mgmt_2) do
				if wpa_key_mgmt == "WPA-PSK" or
				   wpa_key_mgmt == "WPA-PSK-SHA256" then
					table.insert(iface.rsn_override_key_mgmt_2, "FT-PSK")
				elseif wpa_key_mgmt == "WPA-EAP" or
				       wpa_key_mgmt == "WPA-EAP-SHA256" then
					table.insert(iface.rsn_override_key_mgmt_2, "FT-EAP")
				elseif wpa_key_mgmt == "WPA-EAP-SUITE-B-192" then
					table.insert(iface.rsn_override_key_mgmt_2, "FT-EAP-SHA384")
				elseif wpa_key_mgmt == "SAE" then
					table.insert(iface.rsn_override_key_mgmt_2, "FT-SAE")
				elseif wpa_key_mgmt == "SAE-EXT-KEY" then
					table.insert(iface.rsn_override_key_mgmt_2, "FT-SAE-EXT-KEY")
				end
			end
		end

		if iface.ft_psk_generate_local == nil then
			iface.ft_psk_generate_local = "0"
		end

		if iface.mobility_domain == nil then
			local handle = io.popen("echo -n \""..iface.ssid.."\" | md5sum | head -c 4")
			iface.mobility_domain = handle:read("*a"):gsub("[\n\r]", "")
		end

		if iface.ft_over_ds == nil then
			iface.ft_over_ds = "0"
		end

		if iface.reassociation_deadline == nil then
			iface.reassociation_deadline = "1000"
		end

		if iface.ft_psk_generate_local == "0" then
			if iface.r0_key_lifetime == nil then
				iface.r0_key_lifetime = "10000"
			end

			if iface.pmk_r1_push == nil then
				iface.pmk_r1_push = "0"
			end

			if iface.key and (iface.r0kh == nil or iface.r1kh == nil) then
				local handle = io.popen("echo -n \""..iface.mobility_domain.."/"..iface.key.."\" | md5sum | awk '{print $1}'")
				local ft_key = handle:read("*a"):gsub("[\n\r]", "")
				if iface.r0kh == nil then
					iface.r0kh = {}
					table.insert(iface.r0kh, "ff:ff:ff:ff:ff:ff,*,"..ft_key)
				end
				if iface.r1kh == nil then
					iface.r1kh = {}
					table.insert(iface.r1kh, "00:00:00:00:00:00,00:00:00:00:00:00,"..ft_key)
				end
			end
		end
	end

	if iface.wpa_key_mgmt ~= nil then
		if type(iface.wpa_key_mgmt) == "table" and #iface.wpa_key_mgmt > 0 then
			io.write("wpa_key_mgmt="..table.concat(iface.wpa_key_mgmt, " ").."\n")
		elseif type(iface.wpa_key_mgmt) == "string" and iface.wpa_key_mgmt ~= "" then
			io.write("wpa_key_mgmt="..iface.wpa_key_mgmt.."\n")
		end
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
			if iface.wpa == "1" then
				io.write("wpa_pairwise="..table.concat(pairwise, " ").."\n")
			elseif iface.wpa == "2" then
				io.write("rsn_pairwise="..table.concat(pairwise, " ").."\n")
			elseif iface.wpa == "3" then
				io.write("wpa_pairwise="..table.concat(pairwise, " ").."\n")
				io.write("rsn_pairwise="..table.concat(pairwise, " ").."\n")
			end
		end
	end

	if iface.group_cipher and cipher_map[iface.group_cipher]then
		io.write("group_cipher="..cipher_map[iface.group_cipher].."\n")
	end

	if iface.group_mgmt_cipher then
		if iface.group_mgmt_cipher == "aes-128-cmac" then
			io.write("group_mgmt_cipher=AES-128-CMAC\n")
		elseif iface.group_mgmt_cipher == "bip-gmac-256" then
			io.write("group_mgmt_cipher=BIP-GMAC-256\n")
		elseif iface.group_mgmt_cipher == "bip-gmac-128" then
			io.write("group_mgmt_cipher=BIP-GMAC-128\n")
		elseif iface.group_mgmt_cipher == "bip-cmac-256" then
			io.write("group_mgmt_cipher=BIP-CMAC-256\n")
		end
	end

	if (iface.encryption and string.find(iface.encryption, "dpp") ~= nil) or
	   (iface.encryption_override and string.find(iface.encryption_override, "dpp") ~= nil) or
	   (iface.encryption_orverride_2 and string.find(iface.encryption_orverride_2, "dpp") ~= nil) then
		if iface.dpp_pfs ~= nil and
			tonumber(iface.dpp_pfs) >= 0 and
			tonumber(iface.dpp_pfs) <= 2 then
			io.write("dpp_pfs="..iface.dpp_pfs.."\n")
		end
		if iface.dpp_connector ~= nil then
			io.write("dpp_connector="..iface.dpp_connector.."\n")
		end
		if iface.dpp_connector ~= nil then
			io.write("dpp_csign="..iface.dpp_csign.."\n")
		end
		if iface.dpp_netaccesskey ~= nil then
			io.write("dpp_netaccesskey="..iface.dpp_netaccesskey.."\n")
		end
	end

	if string.find(iface.encryption, "wpa") ~= nil then
		iface.ieee8021x = "1"
	end

	if iface.sae_pwe ~= nil and
		tonumber(iface.sae_pwe) >= 0 and
		tonumber(iface.sae_pwe) <= 2 then
		io.write("sae_pwe="..iface.sae_pwe.."\n")
	else
		io.write("sae_pwe=2\n")
	end

	if iface.sae_require_mfp ~= nil then
		io.write("sae_require_mfp="..iface.sae_require_mfp.."\n")
	end

	if iface.sae_groups ~= nil then
		if type(iface.sae_groups) == "string" then
			io.write("sae_groups="..iface.sae_groups.."\n")
		elseif type(iface.sae_groups) == "table" then
			io.write("sae_groups="..table.concat(iface.sae_groups, " ").."\n")
		end
	end

	if iface.owe_transition_bssid ~= nil then
		io.write("owe_transition_bssid="..iface.owe_transition_bssid.."\n")
	end

	if iface.owe_transition_ssid ~= nil then
		io.write("owe_transition_ssid=\""..iface.owe_transition_ssid.."\"\n")
	end

	if iface.owe_transition_ifname ~= nil then
		io.write("owe_transition_ifname="..iface.owe_transition_ifname.."\n")
	end

	if iface.owe_groups ~= nil then
		if type(iface.owe_groups) == "string" then
			io.write("owe_groups="..iface.owe_groups.."\n")
		elseif type(iface.owe_groups) == "table" then
			io.write("owe_groups="..table.concat(iface.owe_groups, " ").."\n")
		end
	end

	if iface.wmm ~= nil and iface.wmm == "0" then
		io.write("wmm_enabled=0\n")
	else
		io.write("wmm_enabled=1\n")
	end

	if iface.rsn_preauth ~= nil then
		io.write("rsn_preauth="..iface.rsn_preauth.."\n")
	else
		io.write("rsn_preauth=0\n")
	end

	if iface.isolate ~= nil then
		io.write("ap_isolate="..iface.isolate.."\n")
	else
		io.write("ap_isolate=0\n")
	end

	if iface.rts ~= nil then
		io.write("rts_threshold="..iface.rts.."\n")
	else
		io.write("rts_threshold=-1\n")
	end

	if iface.frag ~= nil then
		io.write("fragm_threshold="..iface.frag.."\n")
	else
		io.write("fragm_threshold=-1\n")
	end

	if iface.ieee80211w ~= nil and iface.encryption ~= "none" then
		if tonumber(iface.ieee80211w) >= 0 and tonumber(iface.ieee80211w) <= 2 then
			io.write("ieee80211w=", iface.ieee80211w, "\n")
			if tonumber(iface.ieee80211w) >= 1 and tonumber(iface.ieee80211w) <= 2 then
				if iface.beacon_prot ~= nil and iface.beacon_prot == "1" then
					io.write("beacon_prot=1\n")
				end
				if iface.ocv ~= nil and tonumber(iface.ocv) >= 0 and tonumber(iface.ocv) <= 2 then
					io.write("ocv=", iface.ocv, "\n")
				end
			end
		end
	end

	if iface.rsn_override_key_mgmt ~= nil then
		if type(iface.rsn_override_key_mgmt) == "table" and #iface.rsn_override_key_mgmt > 0 then
			io.write("rsn_override_key_mgmt="..table.concat(iface.rsn_override_key_mgmt, " ").."\n")
		elseif type(iface.rsn_override_key_mgmt) == "string" and iface.rsn_override_key_mgmt ~= "" then
			io.write("rsn_override_key_mgmt="..iface.rsn_override_key_mgmt.."\n")
		end
	end

	if iface.rsn_override_pairwise ~= nil then
		local pairwise = {}

		if type(iface.rsn_override_pairwise) == "string" then
			table.insert(pairwise, cipher_map[iface.rsn_override_pairwise])
		elseif type(iface.rsn_override_pairwise) == "table" then
			for i = 1, #iface.rsn_override_pairwise do
				table.insert(pairwise, cipher_map[iface.rsn_override_pairwise[i]])
			end
		end
		io.write("rsn_override_pairwise="..table.concat(pairwise, " ").."\n")
	end

	if iface.rsn_override_mfp ~= nil then
		io.write("rsn_override_mfp="..iface.rsn_override_mfp.."\n")
	end

	if iface.rsn_override_key_mgmt_2 ~= nil then
		if type(iface.rsn_override_key_mgmt_2) == "table" and #iface.rsn_override_key_mgmt_2 > 0 then
			io.write("rsn_override_key_mgmt_2="..table.concat(iface.rsn_override_key_mgmt_2, " ").."\n")
		elseif type(iface.rsn_override_key_mgmt_2) == "string" and iface.rsn_override_key_mgmt_2 ~= "" then
			io.write("rsn_override_key_mgmt_2="..iface.rsn_override_key_mgmt_2.."\n")
		end
	end

	if iface.rsn_override_pairwise_2 ~= nil then
		local pairwise = {}

		if type(iface.rsn_override_pairwise_2) == "string" then
			table.insert(pairwise, cipher_map[iface.rsn_override_pairwise_2])
		elseif type(iface.rsn_override_pairwise_2) == "table" then
			for i = 1, #iface.rsn_override_pairwise_2 do
				table.insert(pairwise, cipher_map[iface.rsn_override_pairwise_2[i]])
			end
		end
		io.write("rsn_override_pairwise_2="..table.concat(pairwise, " ").."\n")
	end

	if iface.rsn_override_mfp_2 ~= nil then
		io.write("rsn_override_mfp_2="..iface.rsn_override_mfp_2.."\n")
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
		if string.match(iface.key, "^[1-4]$") then
			local i
			for i = 0, 3 do
				local key = iface['key'..tostring(i+1)]
				io.write("wep_key", tostring(i), "=", prepare_key_wep(key), "\n")
			end
			local idx = tonumber(iface.key)
			io.write("wep_default_key=", tostring(idx - 1), "\n")
		else
			io.write("wep_key0=", prepare_key_wep(iface.key), "\n")
			io.write("wep_default_key=0\n")
		end
	end
]]

	if iface.key then
		local hex_key = false

		if #iface.key == 64 then
			local i, j = string.find(iface.key, "%x+")
			if i == 1 and j == 64 then
				hex_key = true
			end
		end


		if iface.wpa_key_mgmt then
			for i, key_mgmt in pairs(iface.wpa_key_mgmt) do
				if string.find(key_mgmt, "PSK") ~= nil then
					if hex_key == true then
						if iface.wpa_psk == nil then
							iface.wpa_psk=iface.key
						end
					elseif #iface.key >= 8 and #iface.key <= 63 then
						if iface.wpa_passphrase == nil then
							iface.wpa_passphrase=iface.key
						end
					end
				end
				if string.find(key_mgmt, "SAE") ~= nil then
					if iface.sae_password == nil then
						iface.sae_password=iface.key
					end
				end
			end
		end

		if iface.rsn_override_key_mgmt then
			for i, key_mgmt in pairs(iface.rsn_override_key_mgmt) do
				if string.find(key_mgmt, "PSK") ~= nil then
					if hex_key == true then
						if iface.wpa_psk == nil then
							iface.wpa_psk=iface.key
						end
					elseif #iface.key >= 8 and #iface.key <= 63 then
						if iface.wpa_passphrase == nil then
							iface.wpa_passphrase=iface.key
						end
					end
				end
				if string.find(key_mgmt, "SAE") ~= nil then
					if iface.sae_password == nil then
						iface.sae_password=iface.key
					end
				end
			end
		end

		if iface.rsn_override_key_mgmt_2 then
			for i, key_mgmt in pairs(iface.rsn_override_key_mgmt_2) do
				if string.find(key_mgmt, "PSK") ~= nil then
					if hex_key == true then
						if iface.wpa_psk == nil then
							iface.wpa_psk=iface.key
						end
					elseif #iface.key >= 8 and #iface.key <= 63 then
						if iface.wpa_passphrase == nil then
							iface.wpa_passphrase=iface.key
						end
					end
				end
				if string.find(key_mgmt, "SAE") ~= nil then
					if iface.sae_password == nil then
						iface.sae_password=iface.key
					end
				end
			end
		end
	end

	if iface.wpa_psk ~= nil then
		io.write("wpa_psk=", iface.wpa_psk, "\n")
	end

	if iface.wpa_passphrase ~= nil then
		io.write("wpa_passphrase=", iface.wpa_passphrase, "\n")
	end

	if iface.sae_password ~= nil then
		if type(iface.sae_password) == "string" then
			io.write("sae_password=", iface.sae_password, "\n")
		elseif type(iface.sae_password) == "table" then
			for i = 1, #iface.sae_password do
				io.write("sae_password=", iface.sae_password[i], "\n")
			end
		end
	end

	if iface.rekey_interval ~= nil then
		io.write("wpa_group_rekey=", tostring(iface.rekey_interval), "\n")
	end
	if iface.wpa_strict_rekey ~= nil then
		io.write("wpa_strict_rekey=", tostring(iface.wpa_strict_rekey), "\n")
	end

	if iface.ieee8021x ~= nil and
	   iface.ieee8021x ~= '0' then
		io.write("ieee8021x=", tostring(iface.ieee8021x), "\n")
	end

	if iface.auth_server ~= nil and
	   iface.auth_server ~= '0' then
		io.write("auth_server_addr=", iface.auth_server, "\n")
	else
		io.write("auth_server_addr=127.0.0.1\n")
	end

	if iface.auth_port ~= nil and
	   iface.auth_port ~= '0' then
		io.write("auth_server_port=", iface.auth_port, "\n")
	else
		io.write("auth_server_port=1812\n")
	end

	if iface.auth_secret ~= nil then
		io.write("auth_server_shared_secret=", iface.auth_secret, "\n")
	end

	if iface.acct_server ~= nil and
	   iface.acct_server ~= '0' then
		io.write("acct_server_addr=", iface.acct_server, "\n")

		if iface.acct_port ~= nil and
		   iface.acct_port ~= '0' then
			io.write("acct_server_port=", iface.acct_port, "\n")
		else
			io.write("acct_server_port=1813\n")
		end

		if iface.acct_secret ~= nil then
			io.write("acct_server_shared_secret=", iface.acct_secret, "\n")
		end

		if iface.acct_interim_interval ~= nil then
			io.write("radius_acct_interim_interval=", iface.acct_interim_interval, "\n")
		end
	end

	io.write("ctrl_interface="..var_hostapd_path.."\n")
	if iface.nasid == nil then
		iface.nasid = "ap.mtk.com"
	end
	io.write("nas_identifier="..iface.nasid.."\n")
	io.write("use_driver_iface_addr=1\n")
	io.write("friendly_name=WPS Access Point\n")
	io.write("model_name=MediaTek Wireless Access Point\n")
	io.write("model_number=MT7988\n")
	io.write("serial_number=12345678\n")
	io.write("os_version=80000000\n")

	if iface.ownip then
		io.write("own_ip_addr=", iface.ownip, "\n")
	end

	if string.find(iface.encryption, "psk") ~= nil or
	   string.find(iface.encryption, "none") ~= nil or
	   string.find(iface.encryption, "sae") ~= nil then
		if iface.wps_pin ~= nil then
			io.write("ap_pin=", iface.wps_pin, "\n")
		end
		if iface.ext_registrar ~= nil and iface.ext_registrar == "0" then
			io.write("ap_setup_locked=1\n")
		end

		local wps_config_methods = {"display", "virtual_push_button", "keypad"}
		if iface.wps_label ~= nil then
			if iface.wps_label == "0" then
				for i, config_method in pairs(wps_config_methods) do
					if config_method == "label" then
						table.remove(wps_config_methods, i)
					end
				end
			elseif iface.wps_label == "1" then
				table.insert(wps_config_methods, "label")
			end
		end
		if iface.wps_pushbutton ~= nil then
			if iface.wps_pushbutton == "0" then
				for i, config_method in pairs(wps_config_methods) do
					if config_method == "virtual_push_button" then
						table.remove(wps_config_methods, i)
					end
				end
				for i, config_method in pairs(wps_config_methods) do
					if config_method == "physical_push_button" then
						table.remove(wps_config_methods, i)
					end
			end
			elseif iface.wps_pushbutton == "1" then
				table.insert(wps_config_methods, "physical_push_button")
			end
		end
		if wps_config_methods ~= nil then
			local wps_config_methods_str
			for i, config_method in pairs(wps_config_methods) do
				if wps_config_methods_str then
					wps_config_methods_str = wps_config_methods_str.." "..config_method
				else
					wps_config_methods_str = config_method
				end
			end
			if wps_config_methods_str then
				io.write("config_methods=", wps_config_methods_str, "\n")
			end
		end
		io.write("eapol_key_index_workaround=0\n")
		io.write("eapol_version=2\n")
		io.write("eap_server=1\n")
		if iface.wps_independent == nil then
			iface.wps_independent = "1"
		end
	elseif string.find(iface.encryption, "wpa") ~= nil then
		iface.wps_independent = nil
		iface.wps_state = nil
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("eapol_key_index_workaround=0\n")
			io.write("eapol_version=2\n")
			io.write("eap_server=1\n")
		else
			io.write("eap_server=0\n")
		end
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("eapol_key_index_workaround=0\n")
			io.write("eapol_version=2\n")
			io.write("eap_server=1\n")
		end
	end

	if iface.wps_device_name ~= nil then
		io.write("device_name=", iface.wps_device_name, "\n")
	else
		io.write("device_name=Wireless AP\n")
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

	if dev.map_mode ~= nil and dev.map_mode ~= "0" and dev.band ~= "6G" then
		io.write("wps_rf_bands=ag\n")
	else
		if dev.band == "2.4G" then
			if iface.wps_independent ~= "1" then
				io.write("wps_rf_bands=ag\n")
			else
				io.write("wps_rf_bands=b\n")
			end
		elseif dev.band == "5G" then
			if iface.wps_independent ~= "1" then
				io.write("wps_rf_bands=ag\n")
			else
				io.write("wps_rf_bands=a\n")
			end
		end
	end

	if iface.wps_state ~= nil then
		io.write("wps_state=", iface.wps_state, "\n")
		if iface.wps_state == "1" then
			io.write("upnp_iface=br-", iface.network, "\n")
		end
	else
		if iface.wps_state == nil and dev.map_mode ~= nil and  dev.map_mode ~= "0" then
			io.write("wps_state=2\n")
		else
			io.write("wps_state=0\n")
		end
	end

	if iface.wps_cred_add_sae ~= nil then
		io.write("wps_cred_add_sae=", iface.wps_cred_add_sae, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("wps_cred_add_sae=1\n")
		end
	end

	if dev.map_mode ~= nil and dev.map_mode ~= "0" then
		io.write("wps_independent=0\n")
	else
		if iface.wps_independent ~= nil then
			io.write("wps_independent=", iface.wps_independent, "\n")
		end
	end

	if iface.dpp_pfs ~= nil then
		io.write("dpp_pfs=", iface.dpp_pfs, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("dpp_pfs=2\n")
		end
	end

	if iface.multi_ap ~= nil then
		io.write("multi_ap=", iface.multi_ap, "\n")
	end

	if iface.multi_ap_backhaul_ssid ~= nil then
		io.write("multi_ap_backhaul_ssid=\"", iface.multi_ap_backhaul_ssid, "\"\n")
	end

	if iface.multi_ap_backhaul_key then
		local bh_hex_key = false

		if #iface.multi_ap_backhaul_key == 64 then
			local i, j = string.find(iface.multi_ap_backhaul_key, "%x+")
			if i == 1 and j == 64 then
				bh_hex_key = true
			end
		end

		if bh_hex_key == true then
			io.write("multi_ap_backhaul_wpa_psk=", iface.multi_ap_backhaul_key, "\n")
		else
			if #iface.multi_ap_backhaul_key >= 8 and #iface.multi_ap_backhaul_key <= 63 then
				io.write("multi_ap_backhaul_wpa_passphrase=", iface.multi_ap_backhaul_key, "\n")
			end
		end
	elseif iface.multi_ap_backhaul_wpa_passphrase ~= nil then
		if #iface.multi_ap_backhaul_wpa_passphrase >= 8 and #iface.multi_ap_backhaul_wpa_passphrase <= 63 then
			io.write("multi_ap_backhaul_wpa_passphrase=", iface.multi_ap_backhaul_wpa_passphrase, "\n")
		end
	end

	if iface.multi_ap_backhaul_key_mgmt ~= nil then
		io.write("multi_ap_backhaul_key_mgmt=", iface.multi_ap_backhaul_key_mgmt, "\n")
	end

	if iface.uuid ~= nil then
		io.write("uuid=", iface.uuid, "\n")
	end

	if iface.interworking ~= nil then
		io.write("interworking=", iface.interworking, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("interworking=1\n")
		end
	end

	if iface.mbo ~= nil and iface.mbo == "1" then
		io.write("mbo=1\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("mbo=1\n")
		else
			io.write("mbo=0\n")
		end
	end

	if iface.oce ~= nil and tonumber(iface.oce) >= 0 and tonumber(iface.oce) <= 7 then
		io.write("oce=", iface.oce, "\n")
	else
		io.write("oce=0\n")
	end

	if iface.rrm_neighbor_report ~= nil then
		io.write("rrm_neighbor_report=", iface.rrm_neighbor_report, "\n")
	elseif iface.ieee80211k ~= nil then
		io.write("rrm_neighbor_report=", iface.ieee80211k, "\n")
	elseif dev.map_mode ~= nil and dev.map_mode ~= "0" then
		io.write("rrm_neighbor_report=1\n")
	end

	if iface.rrm_beacon_report ~= nil then
		io.write("rrm_beacon_report=", iface.rrm_beacon_report, "\n")
	elseif iface.ieee80211k ~= nil then
		io.write("rrm_beacon_report=", iface.ieee80211k, "\n")
	elseif dev.map_mode ~= nil and dev.map_mode ~= "0" then
		io.write("rrm_beacon_report=1\n")
	end

	if iface.bss_transition ~= nil then
		io.write("bss_transition=", iface.bss_transition, "\n")
	else
		if dev.map_mode ~= nil and dev.map_mode ~= "0" then
			io.write("bss_transition=1\n")
		end
	end

	if iface.apsd_capable ~= nil then
		io.write("uapsd_advertisement_enabled=", iface.apsd_capable, "\n")
	else
		io.write("uapsd_advertisement_enabled=1")
	end

	if iface.ieee80211r == "1" then
		if iface.mobility_domain ~= nil then
			io.write("mobility_domain="..iface.mobility_domain.."\n")
		end

		if iface.ft_psk_generate_local ~= nil then
			io.write("ft_psk_generate_local="..iface.ft_psk_generate_local.."\n")
		end

		if iface.ft_over_ds ~= nil then
			io.write("ft_over_ds="..iface.ft_over_ds.."\n")
		end

		if iface.reassociation_deadline ~= nil then
			io.write("reassociation_deadline="..iface.reassociation_deadline.."\n")
		end

		if iface.r1_key_holder ~= nil then
			io.write("r1_key_holder="..iface.r1_key_holder.."\n")
		end

		if iface.r0_key_lifetime ~= nil then
			io.write("r0_key_lifetime="..iface.r0_key_lifetime.."\n")
		end

		if iface.pmk_r1_push ~= nil then
			io.write("pmk_r1_push="..iface.pmk_r1_push.."\n")
		end

		if iface.r0kh ~= nil then
			if type(iface.r0kh) == "string" then
				local kh = iface.r0kh:gsub(",", " ")
				io.write("r0kh="..kh.."\n")
			elseif type(iface.r0kh) == "table" then
				for i, r0kh in pairs(iface.r0kh) do
					local kh = r0kh:gsub(",", " ")
					io.write("r0kh="..kh.."\n")
				end
			end
		end

		if iface.r1kh ~= nil then
			if type(iface.r1kh) == "string" then
				local kh = iface.r1kh:gsub(",", " ")
				io.write("r1kh="..kh.."\n")
			elseif type(iface.r1kh) == "table" then
				for i, r1kh in pairs(iface.r1kh) do
					local kh = r1kh:gsub(",", " ")
					io.write("r1kh="..kh.."\n")
				end
			end
		end

		if iface.ft_iface ~= nil then
			io.write("ft_iface="..iface.ft_iface.."\n")
		else
			if iface.network ~= nil then
				io.write("ft_iface=br-"..iface.network.."\n")
			end
		end
	end

	if iface.anqp_elem ~= nil then
		if type(iface.anqp_elem) == "string" then
			local elem = iface.anqp_elem:gsub(",", " ")
			io.write("anqp_elem="..elem.."\n")
		elseif type(iface.anqp_elem) == "table" then
			for i, anqp_elem in pairs(iface.anqp_elem) do
				local elem = anqp_elem:gsub(",", " ")
				io.write("anqp_elem="..elem.."\n")
			end
		end
	end

	if iface.anti_clogging_threshold ~= nil then
		io.write("anti_clogging_threshold="..tostring(iface.anti_clogging_threshold).."\n")
	end

	if iface.transition_disable ~= nil then
		io.write("transition_disable="..tostring(iface.transition_disable).."\n")
	end

	if iface.wpa_ptk_rekey ~= nil then
		io.write("wpa_ptk_rekey="..iface.wpa_ptk_rekey.."\n")
	end

	if iface.wpa_deny_ptk0_rekey ~= nil then
		io.write("wpa_deny_ptk0_rekey="..iface.wpa_deny_ptk0_rekey.."\n")
	end

	if iface.proxy_arp == "1" and
	   iface.network ~= nil and
	   iface.isolate == "1" then
		io.write("proxy_arp=1\n")
	else
		io.write("proxy_arp=0\n")
	end

	io.close()
end


function hostapd_enable_vif(phy, iface)
	local file_name = var_hostapd_path.."hostapd-"..iface[".name"]..".conf"
	local action_wps_er_pid = var_path.."action-"..iface[".name"].."-wps-er.pid"
	local action_wps_er_script = "/lib/wifi/hostapd_wps_er_action.lua"

	if iface.network then
		os.execute("ubus -t 20 wait_for network.interface."..iface.network)
	end
	os.execute(hostapd_cli.." -p "..var_hostapd_path.." -i global raw ADD bss_config="..phy..":"..file_name)
	if iface.wps_state == "1" then
		os.execute("exec 1000>&- && "..hostapd_cli.." -i "..iface[".name"].." -a "..action_wps_er_script.." -B -P "..action_wps_er_pid)
	end
end


function hostapd_disable_vif(iface)
	local file_name = var_hostapd_path.."hostapd-"..iface..".conf"
	local action_wps_er_pid = var_path.."action-"..iface.."-wps-er.pid"

	os.execute("[ -f "..action_wps_er_pid.." ] && kill -SIGTERM `cat "..action_wps_er_pid.."` 2>/dev/null")
	os.execute(hostapd_cli.." -p "..var_hostapd_path.." -i global raw REMOVE "..iface)
	os.remove(action_wps_er_pid)
	os.remove(file_name)
end
