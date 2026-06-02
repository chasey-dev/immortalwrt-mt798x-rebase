#!/usr/bin/env lua

--[[
 * A lua library to manipulate mtk's wifi driver. used in luci-app-mtk.
 *
 * Copyright (C) 2016 Hua Shao <nossiac@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
]]
require("datconf")
require("uci")
local ioctl_help = require "ioctl_helper"
local uciCfgfile = "wireless"
local mtkwifi = {}
local logDisable = 1
local PHY_11BG_MIXED = 0
local PHY_11B = 1
local PHY_11A = 2
local PHY_11ABG_MIXED = 3
local PHY_11G = 4
local PHY_11ABGN_MIXED = 5
local PHY_11N_2_4G = 6
local PHY_11GN_MIXED = 7
local PHY_11AN_MIXED = 8
local PHY_11BGN_MIXED = 9
local PHY_11AGN_MIXED = 10
local PHY_11N_5G = 11
local PHY_11VHT_N_ABG_MIXED = 12
local PHY_11VHT_N_AG_MIXED = 13
local PHY_11VHT_N_A_MIXED = 14
local PHY_11VHT_N_MIXED = 15
local PHY_11AX_24G = 16
local PHY_11AX_5G = 17
local PHY_11AX_6G = 18
local PHY_11AX_24G_6G = 19
local PHY_11AX_5G_6G = 20
local PHY_11AX_24G_5G_6G = 21
local PHY_11BE_24G = 22
local PHY_11BE_5G = 23
local PHY_11BE_6G = 24
local PHY_11BE_24G_6G = 25
local PHY_11BE_5G_6G = 26
local PHY_11BE_24G_5G_6G = 27
local HT_BW_20 = 0
local HT_BW_40 = 1

local VHT_BW_2040 = 0
local VHT_BW_80 = 1
local VHT_BW_160 = 2
local VHT_BW_8080 = 3
local VHT_BW_320 = 4

local EHT_BW_20 = 0
local EHT_BW_2040 = 1
local EHT_BW_80 = 2
local EHT_BW_160 = 3
local EHT_BW_320 = 4
local default_key="11111111"

function mtkwifi.htmode2mode(cfg, htmode, band)
    if htmode ~= nil then
        if string.upper(htmode) == 'NOHT' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                if cfg.pure_11b == '1' then
                    cfg.WirelessMode = PHY_11B
                else
                    cfg.WirelessMode = PHY_11G -- TODO
                end
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11A
            end
        elseif string.upper(htmode) == 'HT20' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11BGN_MIXED
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11AN_MIXED
            end
        elseif string.upper(htmode) == 'HT40' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11BGN_MIXED
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11AN_MIXED
            end
--[[
        elseif string.upper(htmode) == 'HT40+' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11N_2_4G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11N_5G
            end
        elseif string.upper(htmode) == 'HT40-' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11N_2_4G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11N_5G
            end
]]
        elseif string.upper(htmode) == 'VHT20' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11VHT_N_ABG_MIXED -- TODO
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11VHT_N_A_MIXED
            end
        elseif string.upper(htmode) == 'VHT40' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11VHT_N_ABG_MIXED -- TODO
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11VHT_N_A_MIXED
            end
        elseif string.upper(htmode) == 'VHT80' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_80
                cfg.EHT_ApBw = EHT_BW_80
                cfg.WirelessMode = PHY_11VHT_N_A_MIXED
            end
        elseif string.upper(htmode) == 'VHT80_80' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_8080
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11VHT_N_A_MIXED
            end
        elseif string.upper(htmode) == 'VHT160' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11VHT_N_A_MIXED
            end
        elseif string.upper(htmode) == 'HE20' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11AX_24G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11AX_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11AX_6G
            end
        elseif string.upper(htmode) == 'HE40' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11AX_24G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11AX_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11AX_6G
            end
        elseif string.upper(htmode) == 'HE80' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_80
                cfg.EHT_ApBw = EHT_BW_80
                cfg.WirelessMode = PHY_11AX_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_80
                cfg.EHT_ApBw = EHT_BW_80
                cfg.WirelessMode = PHY_11AX_6G
            end
        elseif string.upper(htmode) == 'HE160' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11AX_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11AX_6G
            end
        elseif string.upper(htmode) == 'HE320' then
            if string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11AX_6G
            end
        elseif string.upper(htmode) == 'EHT20' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11BE_24G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11BE_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_20
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_20
                cfg.WirelessMode = PHY_11BE_6G
            end
        elseif string.upper(htmode) == 'EHT40' then
            if string.upper(band) == "2.4G" or
               string.upper(band) == "2G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11BE_24G
            elseif string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11BE_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_2040
                cfg.EHT_ApBw = EHT_BW_2040
                cfg.WirelessMode = PHY_11BE_6G
            end
        elseif string.upper(htmode) == 'EHT80' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_80
                cfg.EHT_ApBw = EHT_BW_80
                cfg.WirelessMode = PHY_11BE_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_80
                cfg.EHT_ApBw = EHT_BW_80
                cfg.WirelessMode = PHY_11BE_6G
            end
        elseif string.upper(htmode) == 'EHT160' then
            if string.upper(band) == "5G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11BE_5G
            elseif string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_160
                cfg.WirelessMode = PHY_11BE_6G
              end
        elseif string.upper(htmode) == 'EHT320' then
            if string.upper(band) == "6G" then
                cfg.HT_BW = HT_BW_40
                cfg.VHT_BW = VHT_BW_160
                cfg.EHT_ApBw = EHT_BW_320
                cfg.WirelessMode = PHY_11BE_6G
            end
        end
    end
    cfg.WirelessMode = tostring(cfg.WirelessMode)
    return cfg
end

function mtkwifi.host2datmode(key_mgmt, wpa, rsn_pairwise)
    local AuthMode, EncType
    if key_mgmt == "WPA-PSK SAE" and wpa == "2" then
        AuthMode = "WPA2PSKWPA3PSK"
    elseif key_mgmt == "WPA-PSK" and wpa == "3" then
        AuthMode = "WPAPSKWPA2PSK"
    elseif key_mgmt == "WPA-PSK" and wpa == "2" then
        AuthMode = "WPA2PSK"
    elseif key_mgmt == "SAE" and wpa == "2" then
        AuthMode = "WPA3PSK"
    end

    if rsn_pairwise == "CCMP" then
        EncType = "AES"
    elseif rsn_pairwise == "CCMP TKIP" then
        EncType = "TKIPAES"
    elseif rsn_pairwise == "TKIP" then
        EncType = "TKIP"
    end

    return AuthMode, EncType
end

function mtkwifi.is_include(value, tab)
    for k,v in ipairs(tab) do
        if v == tostring(value) then
            return false
        end
    end
    return true
end

function debug_write(...)
    -- luci.http.write(...)
    if logDisable == 1 then
         return
    end
    local syslog_msg = "";
    local ff = io.open("/tmp/mtkwifi", "a")
    local nargs = select('#',...)

    for n=1, nargs do
      local v = select(n,...)
      if (type(v) == "string" or type(v) == "number") then
        ff:write(v.." ")
        syslog_msg = syslog_msg..v.." ";
      elseif (type(v) == "boolean") then
        if v then
          ff:write("true ")
          syslog_msg = syslog_msg.."true ";
        else
          ff:write("false ")
          syslog_msg = syslog_msg.."false ";
        end
      elseif (type(v) == "nil") then
        ff:write("nil ")
        syslog_msg = syslog_msg.."nil ";
      else
        ff:write("<Non-printable data type = "..type(v).."> ")
        syslog_msg = syslog_msg.."<Non-printable data type = "..type(v).."> ";
      end
    end
    ff:write("\n")
    ff:close()
    nixio.syslog("debug", syslog_msg)
end

function mtkwifi.get_table_length(T)
    local count = 0
    for _ in pairs(T) do
        count = count + 1
    end
    return count
end

function mtkwifi.get_file_lines(fileName)
    local fd = io.open(fileName, "r")
    if not fd then return end
    local content = fd:read("*all")
    fd:close()
    return mtkwifi.__lines(content)
end

function mtkwifi.__split(s, delimiter)
    if s == nil then s = "0" end
    local result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end

function string:split(sep)
    local sep, fields = sep or ":", {}
    local pattern = string.format("([^%s]+)", sep)
    self:gsub(pattern, function(c) fields[#fields+1] = c end)
    return fields
end

function mtkwifi.__trim(s)
  if s then return (s:gsub("^%s*(.-)%s*$", "%1")) end
end

function mtkwifi.__handleSpecialChars(s)
    s = s:gsub("\\", "\\\\")
    s = s:gsub("\"", "\\\"")
    return s
end

function mtkwifi.__spairs(t, order)
    -- collect the keys
    local keys = {}
    for k in pairs(t) do keys[#keys+1] = k end
    -- if order function given, sort by it by passing the table and keys a, b,
    -- otherwise just sort the keys
    --[[
    if order then
        table.sort(keys, function(a,b) return order(t, a, b) end)
        -- table.sort(keys, order)
    else
        table.sort(keys)
    end
    ]]
    table.sort(keys, order)
    -- return the iterator function
    local i = 0
    return function()
        i = i + 1
        if keys[i] then
            return keys[i], t[keys[i]]
        end
    end
end

function mtkwifi.__lines(str)
    local t = {}
    local function helper(line) table.insert(t, line) return "" end
    helper((str:gsub("(.-)\r?\n", helper)))
    return t
end

function mtkwifi.__get_l1dat()
    if not pcall(require, "l1dat_parser") then
        return
    end

    local parser = require("l1dat_parser")
    local l1dat = parser.load_l1_profile(parser.L1_DAT_PATH)

    return l1dat, parser
end

function mtkwifi.sleep(s)
    local ntime = os.clock() + s
    repeat until os.clock() > ntime
end

function mtkwifi.deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[mtkwifi.deepcopy(orig_key)] = mtkwifi.deepcopy(orig_value)
        end
        setmetatable(copy, mtkwifi.deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

local function __cfg2list(str)
    -- delimeter == ";"
    local i = 1
    local list = {}
    if str == nil then return list end
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list
end

function mtkwifi.token_set(str, n, v)
    -- n start from 1
    -- delimeter == ";"
    if not str then str = "" end
    if not v then v = "" end
    local tmp = __cfg2list(str)
    if type(v) ~= type("") and type(v) ~= type(0) then
        nixio.syslog("err", "invalid value type in token_set, "..type(v))
        return
    end
    if #tmp < tonumber(n) then
        for i=#tmp, tonumber(n) do
            if not tmp[i] then
                tmp[i] = v -- pad holes with v !
            end
        end
    else
        tmp[n] = v
    end
    return table.concat(tmp, ";"):gsub("^;*(.-);*$", "%1"):gsub(";+",";")
end


function mtkwifi.token_get(str, n, v)
    -- n starts from 1
    -- v is the backup in case token n is nil
    if not str then return v end
    local tmp = __cfg2list(str)
    return tmp[tonumber(n)] or v
end

function mtkwifi.read_pipe(pipe)
    local retry_count = 10
    local fp, txt, err
    repeat  -- fp:read() may return error, "Interrupted system call", and can be recovered by doing it again
        fp = io.popen(pipe)
        txt, err = fp:read("*a")
        fp:close()
        retry_count = retry_count - 1
    until err == nil or retry_count == 0
    return txt
end

function mtkwifi.uci_decode_wireless(fileName)
    local file = io.open("/etc/config/"..fileName, "r")
    if file then
        file:close()
    else
        local t = {}
        t["wifi-device"] = {}
        t["wifi-iface"] = {}
        t["wifi-mld"] = {}
        return t
    end
    x = uci.cursor()

    local t = x:get_all(fileName)

    t["wifi-device"] = {}
    t["wifi-iface"] = {}
    t["wifi-mld"] = {}

    x:foreach(fileName, "wifi-device", function(s)
        table.insert(t["wifi-device"], s)
    end)
    x:foreach(fileName, "wifi-iface", function(s)
        table.insert(t["wifi-iface"], s)
    end)
    x:foreach(fileName, "wifi-mld", function(s)
        table.insert(t["wifi-mld"], s)
    end)

    return t
end

function mtkwifi.uci_load_wireless(fileName)
    local uci_cfg
    uci_cfg = mtkwifi.uci_decode_wireless(fileName)
    return uci_cfg
end

function mtkwifi.detect_triband()
    local devs = mtkwifi.get_all_devs()
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1.DEV_RINDEX
    local main_ifname
    local bands = 0
    for _,dev in ipairs(devs) do
        main_ifname = l1dat and l1dat[dridx][dev.devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
        if mtkwifi.exists("/sys/class/net/"..main_ifname) then
            bands = bands + 1
        end
    end
    return bands
end

function mtkwifi.detect_6G()
    local devs = mtkwifi.get_all_devs()
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX
    local isinclude_6G = false
    for _, dev in ipairs(devs) do
        local devname = dev.devname
        local main_ifname = l1dat and l1dat[dridx][devname].main_ifname
        local phy = mtkwifi.iface_type(main_ifname)
        local band = mtkwifi.get_band(phy)

        if band == "6G" then
            isinclude_6G = true
        end
    end
    return isinclude_6G
end

function mtkwifi.get_phy_by_main_ifname(main_ifname)
    local phyfile
    local phyname

    phyfile=io.open("/sys/class/net/"..main_ifname.."/phy80211/name")
    phyname=phyfile:read "*a"
    io.close(phyfile)
    if phyname then
        return (phyname:gsub("^%s*(.-)%s*$", "%1"))
    else
        return nil
    end
end

function mtkwifi.ax_board()
    local include_ax = false
    local fp = io.popen("cat /etc/wireless/l1profile.dat | grep profile_path")
    local result = fp:read("*all")
    fp:close()
    if not result then
        include_ax = false
    end

    if string.find(result, "ax") then
        include_ax = true
    end

    return include_ax
end

function mtkwifi.is_kite()
    local is_kite = false
    local fp = io.popen("cat /etc/wireless/l1profile.dat | grep -E '7992|7993'")
    local result = fp:read("*all")
    fp:close()
    if not result then
        is_kite = false
    end

    if string.find(result, "7992") or string.find(result, "7993") then
        is_kite = true
    end

    return is_kite
end

function mtkwifi.includes_stamld()
    local includes_stamld = false
    local fp = io.popen("cat /etc/config/wireless | grep stamld")
    local result = fp:read("*all")
    fp:close()
    if not result then
        includes_stamld = false
    end

    if string.find(result, "stamld") then
        includes_stamld = true
    end

    return includes_stamld
end

function mtkwifi.get_bawinsize(wireless_mode)
    local bawinsize
    if tonumber(wireless_mode) >= PHY_11BE_24G and
       tonumber(wireless_mode) <= PHY_11BE_24G_5G_6G then
        bawinsize = "1024"
    elseif tonumber(wireless_mode) >= PHY_11AX_24G and
       tonumber(wireless_mode) <= PHY_11AX_24G_5G_6G then
        bawinsize = "256"
    else
        bawinsize = "64"
    end
    return bawinsize
end

function mtkwifi.detect_first_card()
    local devs = mtkwifi.get_all_devs()
    local first_card_profile

    for i,dev in ipairs(devs) do
        first_card_profile = dev.devname
        if i == 1 then break end
    end

    return first_card_profile
end

function mtkwifi.get_up_intf_list(str)
    local ap, sta
    if string.sub(str, 1, 2) == "ra" then
        ap = str
        sta = "apcli"..string.sub(str, 3)
    else
        ap = "ra"..string.sub(str, 6)
        sta = str
    end
    intf_list = {}
    local fp = io.popen("ifconfig | grep '^"..ap.."[0-9]'")
    local result = fp:read("*all")
    fp:close()
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        if line and line ~= '' then
            intf = string.split(line," ")[1]
            table.insert(intf_list, intf)
        end
    end

    local fp = io.popen("ifconfig | grep '"..sta.."[0-9]'")
    local result = fp:read("*all")
    fp:close()
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        if line and line ~= '' then
            intf = string.split(line," ")[1]
            table.insert(intf_list, intf)
        end
    end
    return intf_list
end

function mtkwifi.includes_sta(str)
    includes_sta = false
    local ap, sta
    if string.sub(str, 1, 2) == "ra" then
        ap = str
        sta = "apcli"..string.sub(str, 3)
    else
        ap = "ra"..string.sub(str, 6)
        sta = str
    end

    local fp = io.popen("ifconfig | grep '"..sta.."[0-9]'")
    local result = fp:read("*all")
    fp:close()
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        if line and line ~= '' then
            includes_sta = true
        end
    end
    return includes_sta
end

local function get_vifs_by_dev(ucicfg, devname)
    local vifs = {}
    for vifname, vif in pairs(ucicfg["wifi-iface"]) do
        if vif.device == devname and vif.mode == 'ap' then
            if tonumber(vif.vifidx) then
                vifs[tonumber(vif.vifidx)] = vif
            end
        end
    end

    for vifname, vif in pairs(ucicfg["wifi-iface"]) do
        if vif.device == devname and vif.mode == 'ap' then
            if tonumber(vif.vifidx) == nil  then
                vifs[#vifs+1] = vif
            end
        end
    end

    return vifs
end

function mtkwifi.get_banner()
    local fp = io.popen("cat /etc/banner | grep OpenWrt")
    local result = fp:read("*all")
    fp:close()
    if result == "" or result == nil then return false end
    if string.find(result,"21.02") or string.find(result,"2102") then
        return true
    end
    return false
end

function mtkwifi.load_profile(path, raw)
    local cfgs = {}

    cfgobj = datconf.openfile(path)
    if cfgobj then
        cfgs = cfgobj:getall()
        cfgobj:close()
    elseif raw then
        cfgs = datconf.parse(raw)
    end

    return cfgs
end

local function uci2dat_encryption(encryption)
    local auth = ""
    local encr = ""

    if encryption == "none" then
        auth = "OPEN"
        encr = "NONE"
     elseif encryption == "wep-open" then
        auth = "OPEN"
        encr = "WEP"
     elseif encryption == "owe" then
        auth = "OWE"
        encr = "AES"
     elseif encryption == "wep-shared" then
        auth = "SHARED"
        encr = "WEP"
     elseif encryption == "wep-auto" then
        auth = "WEPAUTO"
        encr = "WEP"
     elseif encryption == "wpa2+tkip" then
        auth = "WPA2"
        encr = "TKIP"
     elseif encryption == "wpa2+tkip+ccmp" then
        auth = "WPA2"
        encr = "TKIPAES"
     elseif encryption == "wpa2+ccmp" then
        auth = "WPA2"
        encr = "AES"
     elseif encryption == "wpa3" then
        auth = "WPA3"
        encr = "AES"
     elseif encryption == "wpa3-192" then
        auth = "WPA3-192"
        encr = "GCMP256"
    elseif encryption == "psk+ccmp" then
        auth = "WPAPSK"
        encr = "AES"
    elseif encryption == "psk+tkip" then
        auth = "WPAPSK"
        encr = "TKIP"
    elseif encryption == "psk+tkip+ccmp" then
        auth = "WPAPSK"
        encr = "TKIPAES"
     elseif encryption == "psk2+ccmp" then
        auth = "WPA2PSK"
        encr = "AES"
     elseif encryption == "psk2+tkip" then
        auth = "WPA2PSK"
        encr = "TKIP"
     elseif encryption == "psk2+tkip+ccmp" then
        auth = "WPA2PSK"
        encr = "TKIPAES"
     elseif encryption == "sae" then
        auth = "WPA3PSK"
        encr = "NONE"
    elseif encryption == "sae+ccmp" then
        auth = "WPA3PSK"
        encr = "AES"
    elseif encryption == "sae+gcmp256" then
        auth = "WPA3PSK"
        encr = "GCMP256"
     elseif encryption == "psk-mixed+tkip" then
        auth = "WPAPSKWPA2PSK"
        encr = "TKIP"
     elseif encryption == "psk-mixed+tkip+ccmp" then
        auth = "WPAPSKWPA2PSK"
        encr = "TKIPAES"
     elseif encryption == "psk-mixed+ccmp" then
        auth = "WPAPSKWPA2PSK"
        encr = "AES"
     elseif encryption == "sae-mixed" then
        auth = "WPA2PSKWPA3PSK"
        encr = "NONE"
     elseif encryption == "wpa-mixed+tkip" then
        auth = "WPA1WPA2"
        encr = "TKIP"
     elseif encryption == "wpa-mixed+ccmp" then
        auth = "WPA1WPA2"
        encr = "AES"
     elseif encryption == "wpa-mixed+tkip+ccmp" then
        auth = "WPA1WPA2"
        encr = "TKIPAES"
     elseif encryption == "sae-compat" then
        auth = "WPA3PSKCompt"
        encr = "NONE"
     elseif encryption == "wpa3-mixed" then
        auth = "WPA3WPA2"
        encr = "AES"
     elseif encryption == "owe" then
        auth = "OWE"
        encr = "AES"
     elseif encryption == "dpp" then
        auth = "DPP"
        encr = "AES"
     elseif encryption == "sae-dpp" then
        auth = "SAE DPP"
        encr = "AES"
     elseif encryption == "psk2-dpp" then
        auth = "WPAPSK DPP"
        encr = "AES"
     elseif encryption == "psk2-sae-dpp" then
        auth = "WPAPSK SAE DPP"
        encr = "AES"
     elseif encryption == "sae-ext+gcmp256" then
        auth = "AKM24"
        encr = "GCMP256"
     elseif encryption ~= "" then
        auth = encryption
        encr = "NONE"
     end

    return auth, encr
end

local function dat2uci_encryption(auth, encr)
    local encryption

    if auth == "OPEN" and encr == "NONE" then
        encryption = "none"
    elseif auth == "OPEN" and encr == "WEP" then
        encryption = "wep-open"
    elseif auth == "OWE" and encr == "AES" then
        encryption = "owe"
    elseif auth == "SHARED" and encr == "WEP" then
        encryption = "wep-shared"
    elseif auth == "WEPAUTO" and encr == "WEP" then
        encryption = "wep-auto"
    elseif auth == "WPA2" and encr == "TKIP" then
        encryption = "wpa2+tkip"
    elseif auth == "WPA2" and encr == "TKIPAES" then
        encryption = "wpa2+tkip+ccmp"
    elseif auth == "WPA2" and encr == "AES" then
        encryption = "wpa2+ccmp"
    elseif auth == "WPA3" and encr == "AES" then
        encryption = "wpa3"
    elseif auth == "WPA3-192" and encr == "GCMP256"  then
        encryption = "wpa3-192"
    elseif auth == "WPAPSK" and encr == "AES" then
        encryption = "psk+ccmp"
    elseif auth == "WPAPSK" and encr == "TKIP" then
        encryption = "psk+tkip"
    elseif auth == "WPAPSK" and encr == "TKIPAES" then
        encryption = "psk+tkip+ccmp"
    elseif auth == "WPA2PSK" and encr == "AES" then
        encryption = "psk2+ccmp"
    elseif auth == "WPA2PSK" and encr == "TKIP" then
        encryption = "psk2+tkip"
    elseif auth == "WPA2PSK" and encr == "TKIPAES" then
        encryption = "psk2+tkip+ccmp"
    elseif auth == "WPA3PSK" and encr == "NONE" then
        encryption = "sae"
    elseif auth == "WPA3PSK" and encr == "AES" then
        encryption = "sae+ccmp"
    elseif auth == "WPA3PSK" and encr == "GCMP256" then
        encryption = "sae+gcmp256"
    elseif auth == "WPAPSKWPA2PSK" and encr == "TKIP" then
        encryption = "psk-mixed+tkip"
    elseif auth == "WPAPSKWPA2PSK" and encr == "TKIPAES" then
        encryption = "psk-mixed+tkip+ccmp"
    elseif auth == "WPAPSKWPA2PSK" and encr == "AES" then
        encryption = "psk-mixed+ccmp"
    elseif auth == "WPA2PSKWPA3PSK" and encr == "NONE" then
        encryption = "sae-mixed"
    elseif auth == "WPA1WPA2" and encr == "TKIP" then
        encryption = "wpa-mixed+tkip"
    elseif auth == "WPA1WPA2" and encr == "AES" then
        encryption = "wpa-mixed+ccmp"
    elseif auth == "WPA1WPA2" and encr == "TKIPAES" then
        encryption = "wpa-mixed+tkip+ccmp"
    elseif auth == "WPA3WPA2" then
        encryption = "wpa3-mixed"
    elseif auth == "OWE" and encr == "AES" then
        encryption = "owe"
    elseif auth == "DPP" and encr == "AES" then
        encryption = "dpp"
    elseif auth == "SAE DPP" and encr == "AES" then
        encryption = "sae-dpp"
    elseif auth == "WPAPSK DPP" and encr == "AES" then
        encryption = "psk2-dpp"
    elseif auth == "WPAPSK SAE DPP" and encr == "AES" then
        encryption = "psk2-sae-dpp"
    elseif auth == "WPA3PSKCompt" and encr == "NONE" then
        encryption = "sae-compat"
    elseif auth == "AKM24" and encr == "GCMP256" then
        encryption = "sae-ext+gcmp256"
    elseif auth ~= "" and encr == "NONE" then
        encryption = auth
    elseif auth == "" then
        encryption = "sae"
    end

    return encryption
end

local function cfg2dev(cfg, devname, dev)
    assert(cfg ~= nil)
    assert(dev ~= nil)

    dev[".name"] = string.gsub(devname, "%.", "_")
    dev.type = "mtkwifi"
    dev.vendor = "mediatek"
    dev.txpower = cfg.TxPower
    dev.percentag_enable = cfg.PERCENTAGEenable
    if cfg.Channel == "0" then
        dev.channel = "auto"
        if cfg.AutoChannelSelect then
            dev.acs_alg = cfg.AutoChannelSelect
        end
    else
    dev.channel = cfg.Channel
    end
    dev.acs_skiplist = cfg.AutoChannelSkipList

    dev.channel_grp = cfg.ChannelGrp
    dev.autoch = cfg.AutoChannelSelect
    dev.beacon_int = cfg.BeaconPeriod
    dev.txpreamble = cfg.TxPreamble

    dev.htmode = cfg.Htmode
    dev.ht_extcha = (cfg.HT_EXTCHA or ""):split(";")[1]


    local band = mtkwifi.band(string.split(cfg.WirelessMode,";")[1])
    if dev.htmode == "EHT320"and dev.ht_extcha == "1" then
        dev.htmode = "EHT320-2"
    elseif dev.htmode == "HT40" and band == "2.4G" then
        if dev.ht_extcha == "1" then
            dev.htmode = "HT40+"
        else
            dev.htmode = "HT40-"
        end
    end

    dev.pure_11b = cfg.pure_11b
    dev.ht_coex = cfg.HT_BSSCoexistence

    dev.vht_sec80_channel = cfg.VHT_Sec80_Channel
    dev.ht_txstream = cfg.HT_TxStream
    dev.ht_rxstream = cfg.HT_RxStream
    dev.shortslot = cfg.ShortSlot
    dev.ht_distkip = cfg.HT_DisallowTKIP
    dev.bgprotect = cfg.BGProtection
    dev.txburst = cfg.TxBurst
    dev.band = dev.band

    if dev.band == "2.4G" then
        dev.region = cfg.CountryRegion
    else
        dev.aregion = cfg.CountryRegionABand
    end

    dev.pktaggregate = cfg.PktAggregate
    dev.country = cfg.CountryCode
    dev.ht_mcs = cfg.HT_MCS
    dev.e2p_accessmode = cfg.E2pAccessMode
    dev.map_mode = cfg.MapMode
    dev.dbdc_mode = cfg.DBDC_MODE
    dev.etxbfencond = cfg.ETxBfEnCond
    dev.itxbfen = cfg.ITxBfEn
    dev.mutxrx_enable = cfg.MUTxRxEnable
    dev.bss_color = cfg.BSSColorValue
    dev.colocated_bssid = cfg.CoLocatedBSSID
    dev.twt_support = cfg.TWTSupport
    dev.individual_twt_support = cfg.IndividualTWTSupport
    dev.he_ldpc = cfg.HE_LDPC
    dev.txop = cfg.TxOP
    dev.doth = cfg.IEEE80211H
    dev.dfs_enable = cfg.DfsEnable
    dev.sr_mode = cfg.SRMode
    dev.sre_enable = cfg.SREnable
    dev.powerup_enbale = cfg.PowerUpenable
    dev.powerup_cckofdm = cfg.PowerUpCckOfdm
    dev.powerup_ht20 = cfg.PowerUpHT20
    dev.powerup_ht40 = cfg.PowerUpHT40
    dev.powerup_vht20 = cfg.PowerUpVHT20
    dev.powerup_vht40 = cfg.PowerUpVHT40
    dev.powerup_vht80 = cfg.PowerUpVHT80
    dev.powerup_vht160 = cfg.PowerUpVHT160

    dev.vow_airtime_fairness_en = cfg.VOW_Airtime_Fairness_En
    dev.ht_rdg = cfg.HT_RDG
    dev.whnat = cfg.WHNAT
    dev.e2p_accessmode = cfg.E2pAccessMode
    dev.vow_bw_ctrl = cfg.VOW_BW_Ctrl
    dev.vow_ex_en = cfg.VOW_RX_En

    dev.dfs_zero_wait = cfg.DfsZeroWait
    dev.dfs_dedicated_zero_wait = cfg.DfsDedicatedZeroWait
    dev.dfs_zero_wait_default = cfg.DfsZeroWaitDefault
    dev.rd_region = cfg.RDRegion
    dev.psc_acs = cfg.PSC_ACS
    dev.dabs_group_key_bitmap = cfg.DABSgroupkeybitmap
    dev.dabs_vendor_key_bitmap = cfg.DABSvendorkeybitmap
    dev.qos_enable = cfg.QoSEnable
    dev.dfs_offchannel_cac = cfg.OcacEnable or "0"

    return dev
end

local function cfg2iface(cfg, devname, ifname, iface, i)
    assert(cfg ~= nil)
    assert(iface ~= nil)

    local encr_list = cfg.EncrypType:split()
    local auth_list = cfg.AuthMode:split()
    encr_list = (encr_list[1] or ""):split(";")
    auth_list = (auth_list[1] or ""):split(";")

    iface[".name"] = ifname
    iface.device = devname
    iface.mode = "ap"
    iface.disabled = mtkwifi.token_get(cfg.ApEnable, i, mtkwifi.__split(cfg.ApEnable,";")[1])
    iface.ssid = cfg["SSID"..tostring(i)]
    iface.network = "lan"
    iface.vifidx = i
    iface.hidden = mtkwifi.token_get(cfg.HideSSID, i, 0)
    iface.wmm = mtkwifi.token_get(cfg.WmmCapable, i, mtkwifi.__split(cfg.WmmCapable,";")[1])
    iface.dtim_period = mtkwifi.token_get(cfg.DtimPeriod, i, mtkwifi.__split(cfg.DtimPeriod,";")[1])

    iface.encryption = dat2uci_encryption(auth_list[i], encr_list[i])
    iface.key = ""
    if encr_list[i] == "WEP" then
        iface.key = mtkwifi.token_get(cfg.DefaultKeyID, i, mtkwifi.__split(cfg.DefaultKeyID,";")[1])
    elseif auth_list[i] == "WPA2PSK" or auth_list[i] == "WPA3PSK" or
        auth_list[i] == "WPAPSKWPA2PSK" or auth_list[i] == "WPA2PSKWPA3PSK" or
        auth_list[i] == "WPAPSK SAE DPP" or auth_list[i] == "SAE DPP" or
        auth_list[i] == "WPAPSK DPP" then
        iface.key = cfg["WPAPSK"..tostring(i)]
    elseif auth_list[i] ~= "" or encr_list[i] == "" then
        iface.key = cfg["WPAPSK"..tostring(i)]
    end

    local j
    for j = 1, 4 do
        iface["key"..tostring(j)] = cfg["Key"..tostring(j).."Str"..tostring(i)] or ''
    end

    iface.ieee8021x = mtkwifi.token_get(cfg.IEEE8021X, i, mtkwifi.__split(cfg.IEEE8021X,";")[1])
    iface.auth_server = mtkwifi.token_get(cfg.RADIUS_Server, i)
    iface.auth_port = mtkwifi.token_get(cfg.RADIUS_Port, i)
    iface.auth_secret = cfg["RADIUS_Key"..tostring(i)]
    iface.ownip = mtkwifi.token_get(cfg.own_ip_addr, i)
    iface.idle_timeout = cfg.idle_timeout_interval
    iface.session_timeout = mtkwifi.token_get(cfg.session_timeout_interval, i, mtkwifi.__split(cfg.session_timeout_interval,";")[1])

    local pmfmfpc = mtkwifi.token_get(cfg.PMFMFPC, i, mtkwifi.__split(cfg.PMFMFPC,";")[1])
    local pmfmfpr = mtkwifi.token_get(cfg.PMFMFPR, i, mtkwifi.__split(cfg.PMFMFPR,";")[1])

    if pmfmfpc == '1' and pmfmfpr == '1' then
        iface.ieee80211w = '2'
    elseif pmfmfpc == '1' then
        iface.ieee80211w = '1'
    else
        iface.ieee80211w = '0'
    end

    iface.pmf_sha256 = mtkwifi.token_get(cfg.PMFSHA256, i, mtkwifi.__split(cfg.PMFSHA256,";")[1])

    iface.pmk_cache_period = mtkwifi.token_get(cfg.PMKCachePeriod, i, mtkwifi.__split(cfg.PMKCachePeriod ,";")[1])
    iface.rekey_interval = mtkwifi.token_get(cfg.RekeyInterval, i, mtkwifi.__split(cfg.RekeyInterval,";")[1])
    iface.rekey_meth = mtkwifi.token_get(cfg.RekeyMethod, i, mtkwifi.__split(cfg.RekeyMethod,";")[1])
    iface.rsn_preauth = mtkwifi.token_get(cfg.PreAuth, i, mtkwifi.__split(cfg.PreAuth,";")[1])

    iface.tx_rate = mtkwifi.token_get(cfg.TxRate, i, mtkwifi.__split(cfg.TxRate,";")[1])
    iface.isolate = mtkwifi.token_get(cfg.NoForwarding , i, mtkwifi.__split(cfg.NoForwarding ,";")[1])
    iface.rts = mtkwifi.token_get(cfg.RTSThreshold, i, mtkwifi.__split(cfg.RTSThreshold,";")[1])
    iface.frag = mtkwifi.token_get(cfg.FragThreshold, i, mtkwifi.__split(cfg.FragThreshold,";")[1])
    iface.apsd_capable = mtkwifi.token_get(cfg.APSDCapable, i, mtkwifi.__split(cfg.APSDCapable,";")[1])
    iface.vht_bw_signal = mtkwifi.token_get(cfg.VHT_BW_SIGNAL, i, mtkwifi.__split(cfg.VHT_BW_SIGNAL,";")[1])
    iface.vht_ldpc = mtkwifi.token_get(cfg.VHT_LDPC, i, mtkwifi.__split(cfg.VHT_LDPC,";")[1])
    iface.vht_stbc = mtkwifi.token_get(cfg.VHT_STBC, i, mtkwifi.__split(cfg.VHT_STBC,";")[1])
    iface.vht_sgi = mtkwifi.token_get(cfg.VHT_SGI, i, mtkwifi.__split(cfg.VHT_SGI,";")[1])
    iface.ht_ldpc = mtkwifi.token_get(cfg.HT_LDPC, i, mtkwifi.__split(cfg.HT_LDPC,";")[1])
    iface.ht_stbc = mtkwifi.token_get(cfg.HT_STBC, i, mtkwifi.__split(cfg.HT_STBC,";")[1])
    iface.ht_protect = mtkwifi.token_get(cfg.HT_PROTECT, i, mtkwifi.__split(cfg.HT_PROTECT,";")[1])
    iface.ht_gi = mtkwifi.token_get(cfg.HT_GI , i, mtkwifi.__split(cfg.HT_GI ,";")[1])
    iface.ht_opmode = mtkwifi.token_get(cfg.HT_OpMode , i, mtkwifi.__split(cfg.HT_OpMode ,";")[1])
    iface.ht_amsdu = mtkwifi.token_get(cfg.HT_AMSDU, i, mtkwifi.__split(cfg.HT_AMSDU,";")[1])
    iface.ht_autoba = mtkwifi.token_get(cfg.HT_AutoBA , i, mtkwifi.__split(cfg.HT_AutoBA ,";")[1])
    iface.ht_bawinsize = mtkwifi.token_get(cfg.HT_BAWinSize , i, mtkwifi.__split(cfg.HT_BAWinSize ,";")[1])
    iface.ht_badec = mtkwifi.token_get(cfg.HT_BADecline, i, mtkwifi.__split(cfg.HT_BADecline,";")[1])
    iface.igmpsn_enable = mtkwifi.token_get(cfg.IgmpSnEnable, i, mtkwifi.__split(cfg.IgmpSnEnable,";")[1])

    iface.mumimoul_enable = mtkwifi.token_get(cfg.MuMimoUlEnable, i, mtkwifi.__split(cfg.MuMimoUlEnable,";")[1])
    iface.mumimodl_enable = mtkwifi.token_get(cfg.MuMimoDlEnable, i, mtkwifi.__split(cfg.MuMimoDlEnable,";")[1])
    iface.muofdmaul_enable = mtkwifi.token_get(cfg.MuOfdmaUlEnable, i, mtkwifi.__split(cfg.MuOfdmaUlEnable,";")[1])
    iface.muofdmadl_enable = mtkwifi.token_get(cfg.MuOfdmaDlEnable, i, mtkwifi.__split(cfg.MuOfdmaDlEnable,";")[1])
    iface.vow_group_max_ratio = mtkwifi.token_get(cfg.VOW_Group_Max_Ratio, i, mtkwifi.__split(cfg.VOW_Group_Max_Ratio,";")[1])
    iface.vow_group_min_ratio = mtkwifi.token_get(cfg.VOW_Group_Min_Ratio, i, mtkwifi.__split(cfg.VOW_Group_Min_Ratio,";")[1])
    iface.vow_airtime_ctrl_en = mtkwifi.token_get(cfg.VOW_Airtime_Ctrl_En, i, mtkwifi.__split(cfg.VOW_Airtime_Ctrl_En,";")[1])
    iface.vow_group_max_rate = mtkwifi.token_get(cfg.VOW_Group_Max_Rate , i, mtkwifi.__split(cfg.VOW_Group_Max_Rate ,";")[1])
    iface.vow_group_min_rate = mtkwifi.token_get(cfg.VOW_Group_Min_Rate, i, mtkwifi.__split(cfg.VOW_Group_Min_Rate,";")[1])
    iface.vow_rate_ctrl_en = mtkwifi.token_get(cfg.VOW_Rate_Ctrl_En, i, mtkwifi.__split(cfg.VOW_Rate_Ctrl_En,";")[1])

    iface.wds = mtkwifi.token_get(cfg.WdsEnable, i, mtkwifi.__split(cfg.WdsEnable,";")[1])
    iface.wdslist = cfg.WdsList
    iface.wds0key = cfg.Wds0Key
    iface.wds1key = cfg.Wds1Key
    iface.wds2key = cfg.Wds2Key
    iface.wds3key = cfg.Wds3Key
    iface.wdsencryptype = cfg.WdsEncrypType
    iface.wdsphymode = cfg.WdsPhyMode

    local wsc_confmode, wsc_confstatus
    wsc_confmode = mtkwifi.token_get(cfg.WscConfMode, i, mtkwifi.__split(cfg.WscConfMode,";")[1])
    wsc_confmode = tonumber(wsc_confmode)
    wsc_confstatus = mtkwifi.token_get(cfg.WscConfStatus, i, mtkwifi.__split(cfg.WscConfStatus,";")[1])
    wsc_confstatus = tonumber(wsc_confstatus)

    iface.wps_state = ''
    if wsc_confmode and wsc_confmode ~= 0 then
        if wsc_confstatus == 1 then
            iface.wps_state = '1'
        elseif wsc_confstatus == 2 then
            iface.wps_state = '2'
        end
    end

    iface.wps_pin = mtkwifi.token_get(cfg.WscVendorPinCode, i, mtkwifi.__split(cfg.WscVendorPinCode,";")[1])

    iface.access_policy = cfg["AccessPolicy"..tostring(i-1)]
    iface.access_list = __cfg2list(cfg["AccessControlList"..tostring(i-1)])

    iface.multi_ap_backhaul_ssid = mtkwifi.token_get(cfg.MultiApBacSsid, i, mtkwifi.__split(cfg.MultiApBacSsid,";")[1])
    iface.multi_ap_backhaul_wpa_passphrase = mtkwifi.token_get(cfg.MultiApBacPass, i, mtkwifi.__split(cfg.MultiApBacPass,";")[1])
    iface.auth_algs = mtkwifi.token_get(cfg.AuthAlgs, i, mtkwifi.__split(cfg.AuthAlgs,";")[1])
    iface.multi_ap = mtkwifi.token_get(cfg.MultiAp, i, mtkwifi.__split(cfg.MultiAp,";")[1])
    iface.ignore_broadcast_ssid = mtkwifi.token_get(cfg.IgnoreBroadSsid, i, mtkwifi.__split(cfg.IgnoreBroadSsid,";")[1])
    iface.wps_cred_add_sae = mtkwifi.token_get(cfg.WpsCred, i, mtkwifi.__split(cfg.WpsCred,";")[1])
    iface.wps_independent = mtkwifi.token_get(cfg.WpsInde, i, mtkwifi.__split(cfg.WpsInde,";")[1])
    local mbo = 1
    if tonumber(cfg.MapMode) == 0 then
        mbo = 0
    end
    iface.mbo = mbo
    iface.dpp_pfs = mtkwifi.token_get(cfg.DppPfs, i, mtkwifi.__split(cfg.DppPfs,";")[1])
    iface.interworking = mtkwifi.token_get(cfg.Interwork, i, mtkwifi.__split(cfg.Interwork,";")[1])
    iface.sae_pwe = mtkwifi.token_get(cfg.SaePwe, i, mtkwifi.__split(cfg.SaePwe,";")[1])
    iface.eml_mode = mtkwifi.token_get(cfg.EHT_ApEmlsr_mr, i, "0")
    iface.eml_trans_to = mtkwifi.token_get(cfg.EHT_ApEmlsr_mr_trans_to, i, "0")
    iface.eml_omn_en = mtkwifi.token_get(cfg.EHT_ApEmlsr_mr_OMN, i, "0")
    iface.mrsno_enable = mtkwifi.token_get(cfg.Mrsno_En, i, "0")

    iface.rrm_beacon_report = cfg.rrm_beacon_report
    iface.bss_transtion = cfg.bss_transtion

    return iface
end

local function cfg2apcli(cfg, devname, ifname, iface, i)
    assert(cfg ~= nil)
    assert(iface ~= nil)

    iface[".name"] = ifname
    iface.device = devname
    iface.mode = "sta"
    if cfg.ApCliEnable == "1" then
        iface.disabled = "0"
    else
        iface.disabled = "1"
    end

    iface.ssid = cfg.ApCliSsid
    iface.bssid = cfg.ApCliBssid

    iface.encryption = dat2uci_encryption(cfg.ApCliAuthMode, cfg.ApCliEncrypType)

    iface.key = ""
    if cfg.ApCliEncrypType == "WEP" then
        iface.key = cfg.ApCliDefaultKeyID
    elseif cfg.ApCliAuthMode == "WPA2PSK" or cfg.ApCliAuthMode == "WPA3PSK" or cfg.ApCliAuthMode == "AKM24" or
        cfg.ApCliAuthMode == "WPAPSKWPA2PSK" or cfg.ApCliAuthMode == "WPA2PSKWPA3PSK" or cfg.ApCliAuthMode == "WPAPSK" then
        iface.key = cfg.ApCliWPAPSK
    end

    if cfg.ApCliPMFMFPC == '1' and cfg.ApCliPMFMFPR == '1' then
        iface.ieee80211w = '2'
    elseif cfg.ApCliPMFMFPC == '1' then
        iface.ieee80211w = '1'
    else
        iface.ieee80211w = '0'
    end

    iface.pmf_sha256 = cfg.ApCliPMFSHA256
    iface.owetrante = cfg.ApCliOWETranIe
    iface.mac_repeateren = cfg.MACRepeaterEn

    local j
    for j = 1, 4 do
        iface["key"..tostring(j)] = cfg["ApCliKey"..tostring(j).."Str"] or ''
    end

    return iface
end

local function dev2cfg(dev, cfg)
    assert(dev ~= nil)
    assert(cfg ~= nil)

    cfg.TxPower = dev.txpower
    cfg.PERCENTAGEenable = dev.percentag_enable
    cfg.Channel = dev.channel

    if dev.channel == nil or
       string.lower(dev.channel) == "auto" or
       dev.channel == "0" then
        cfg.Channel = 0
        if dev.acs_alg then
            cfg.AutoChannelSelect = dev.acs_alg
        else
            cfg.AutoChannelSelect = 3
        end
    elseif tonumber(dev.channel) > 0 then
        cfg.Channel = dev.channel
        cfg.AutoChannelSelect = 0
    end
    cfg.AutoChannelSkipList = dev.acs_skiplist or ""

    cfg.ChannelGrp = dev.channel_grp
    cfg.AutoChannelSelect = dev.autoch
    cfg.BeaconPeriod = dev.beacon_int
    cfg.TxPreamble = dev.txpreamble
    if dev.htmode == "EHT320-2" then
        cfg.Htmode = "EHT320"
        dev.htmode = "EHT320"
    elseif dev.htmode == "HT40-" or dev.htmode == "HT40+" then
        cfg.Htmode = "HT40"
        dev.htmode = "HT40"
    else
        cfg.Htmode = dev.htmode
    end
    cfg.pure_11b = dev.pure_11b
    cfg.HT_BSSCoexistence = dev.ht_coex or "0"

    cfg = mtkwifi.htmode2mode(cfg, dev.htmode, dev.band)

    cfg.HT_EXTCHA = dev.ht_extcha
    cfg.HT_TxStream = dev.ht_txstream
    cfg.HT_RxStream = dev.ht_rxstream
    cfg.ShortSlot = dev.shortslot
    cfg.HT_DisallowTKIP = dev.ht_distkip
    cfg.BGProtection = dev.bgprotect
    cfg.TxBurst = dev.txburst

    if dev.band == "2.4G" then
        cfg.CountryRegion = dev.region
    else
        cfg.CountryRegionABand = dev.aregion
    end

    cfg.PktAggregate = dev.pktaggregate
    cfg.CountryCode = dev.country
    cfg.HT_MCS = dev.ht_mcs
    cfg.E2pAccessMode = dev.e2p_accessmode
    cfg.MapMode = dev.map_mode
    cfg.SRMode = dev.sr_mode
    cfg.DBDC_MODE = dev.dbdc_mode
    cfg.ETxBfEnCond = dev.etxbfencond
    cfg.ITxBfEn = dev.itxbfen
    cfg.MUTxRxEnable = dev.mutxrx_enable
    cfg.BSSColorValue = dev.bss_color
    cfg.CoLocatedBSSID = dev.colocated_bssid
    cfg.TWTSupport = dev.twt_support
    cfg.IndividualTWTSupport = dev.individual_twt_support
    cfg.HE_LDPC = dev.he_ldpc
    cfg.TxOP = dev.txop
    cfg.IEEE80211H = dev.doth
    cfg.DfsEnable = dev.dfs_enable
    cfg.SREnable = dev.sre_enable
    cfg.PowerUpenable = dev.powerup_enbale
    cfg.PowerUpCckOfdm = dev.powerup_cckofdm
    cfg.PowerUpHT20 = dev.powerup_ht20
    cfg.PowerUpHT40 = dev.powerup_ht40
    cfg.PowerUpVHT20 = dev.powerup_vht20
    cfg.PowerUpVHT40 = dev.powerup_vht40
    cfg.PowerUpVHT80 = dev.powerup_vht80
    cfg.PowerUpVHT160 = dev.powerup_vht160

    cfg.VOW_Airtime_Fairness_En = dev.vow_airtime_fairness_en
    cfg.HT_RDG = dev.ht_rdg
    cfg.WHNAT = dev.whnat
    cfg.E2pAccessMode = dev.e2p_accessmode
    cfg.VOW_BW_Ctrl = dev.vow_bw_ctrl
    cfg.VOW_RX_En = dev.vow_ex_en

    cfg.DfsZeroWait = dev.dfs_zero_wait
    cfg.DfsDedicatedZeroWait = dev.dfs_dedicated_zero_wait
    cfg.DfsZeroWaitDefault = dev.dfs_zero_wait_default
    cfg.RDRegion = dev.rd_region
    cfg.PSC_ACS = dev.psc_acs
    cfg.DABSgroupkeybitmap = dev.dabs_group_key_bitmap;
    cfg.DABSvendorkeybitmap = dev.dabs_vendor_key_bitmap;
    cfg.QoSEnable = dev.qos_enable
    cfg.OcacEnable = dev.dfs_offchannel_cac or "0"

    return cfg
end

local function iface2cfg(iface, i, cfg)
    assert(iface ~= nil)
    assert(cfg ~= nil)

    local encr
    local auth

    cfg["SSID"..tostring(i)] = iface.ssid
    cfg.HideSSID = mtkwifi.token_set(cfg.HideSSID, i, iface.hidden)
    cfg.WmmCapable = mtkwifi.token_set(cfg.WmmCapable, i, iface.wmm)
    cfg.DtimPeriod = mtkwifi.token_set(cfg.DtimPeriod, i, iface.dtim_period)
    auth, encr = uci2dat_encryption(iface.encryption)

    cfg.AuthMode = mtkwifi.token_set(cfg.AuthMode, i, auth)
    cfg.EncrypType = mtkwifi.token_set(cfg.EncrypType, i, encr)
    cfg.ApEnable = mtkwifi.token_set(cfg.ApEnable, i, iface.disabled or "0")
    if encr == "WEP" then
        cfg.DefaultKeyID = mtkwifi.token_set(cfg.DefaultKeyID, i, iface.key)
    elseif auth == "WPA2PSK" or auth == "WPA3PSK" or
        auth == "WPAPSKWPA2PSK" or auth == "WPA2PSKWPA3PSK" or
        auth == "WPAPSK SAE DPP" or auth == "SAE DPP" or
        auth == "WPAPSK DPP" then
        cfg["WPAPSK"..tostring(i)] = iface.key
    elseif auth ~= "" or auth == "" then
        cfg["WPAPSK"..tostring(i)] = iface.key or ""
    end

    local j
    for j = 1, 4 do
        local k = iface["key"..tostring(j)] or ''
        local len = #k
        if (len == 10 or len == 26 or len == 32) and k == string.match(k, '%x+') then
            cfg["Key"..tostring(j).."Type"] = mtkwifi.token_set(cfg["Key"..tostring(j).."Type"], i, 0)
        elseif (len == 5 or len == 13 or len == 16) then
            cfg["Key"..tostring(j).."Type"] = mtkwifi.token_set(cfg["Key"..tostring(j).."Type"], i, 1)
        end
        cfg["Key"..tostring(j).."Str"..tostring(i)] = k
    end


    cfg.IEEE8021X = mtkwifi.token_set(cfg.IEEE8021X, i, iface.ieee8021x)
    cfg.RADIUS_Server = mtkwifi.token_set(cfg.RADIUS_Server, i, iface.auth_server)
    cfg.RADIUS_Port = mtkwifi.token_set(cfg.RADIUS_Port, i, iface.auth_port)
    cfg["RADIUS_Key"..tostring(i)] = iface.auth_secret
    cfg.own_ip_addr = mtkwifi.token_set(cfg.own_ip_addr, i, iface.ownip)
    cfg.idle_timeout_interval = iface.idle_timeout
    cfg.session_timeout_interval = mtkwifi.token_set(cfg.session_timeout_interval, i, iface.session_timeout)

    if iface.ieee80211w == '2' then
        cfg.PMFMFPC = mtkwifi.token_set(cfg.PMFMFPC, i, '1')
        cfg.PMFMFPR = mtkwifi.token_set(cfg.PMFMFPR, i, '1')
    elseif iface.ieee80211w == '1' then
        cfg.PMFMFPC = mtkwifi.token_set(cfg.PMFMFPC, i, '1')
        cfg.PMFMFPR = mtkwifi.token_set(cfg.PMFMFPR, i, '0')
    elseif iface.ieee80211w == '0' then
        cfg.PMFMFPC = mtkwifi.token_set(cfg.PMFMFPC, i, '0')
        cfg.PMFMFPR = mtkwifi.token_set(cfg.PMFMFPR, i, '0')
    end

    cfg.PMFSHA256 = mtkwifi.token_set(cfg.PMFSHA256, i, iface.pmf_sha256)
    cfg.PMKCachePeriod = mtkwifi.token_set(cfg.PMKCachePeriod, i, iface.pmk_cache_period )
    cfg.RekeyInterval = mtkwifi.token_set(cfg.RekeyInterval, i, iface.rekey_interval)
    cfg.RekeyMethod = mtkwifi.token_set(cfg.RekeyMethod, i, iface.rekey_meth)
    cfg.PreAuth = mtkwifi.token_set(cfg.PreAuth, i, iface.rsn_preauth)

    cfg.TxRate = mtkwifi.token_set(cfg.TxRate, i, iface.tx_rate)
    cfg.NoForwarding = mtkwifi.token_set(cfg.NoForwarding, i, iface.isolate)
    cfg.VHT_BW_SIGNAL = mtkwifi.token_set(cfg.VHT_BW_SIGNAL, i, iface.vht_bw_signal)
    cfg.VHT_SGI = mtkwifi.token_set(cfg.VHT_SGI, i, iface.vht_sgi)
    cfg.RTSThreshold = mtkwifi.token_set(cfg.RTSThreshold, i, iface.rts)
    cfg.FragThreshold = mtkwifi.token_set(cfg.FragThreshold, i, iface.frag)
    cfg.APSDCapable = mtkwifi.token_set(cfg.APSDCapable, i, iface.apsd_capable)
    cfg.VHT_LDPC = mtkwifi.token_set(cfg.VHT_LDPC, i, iface.vht_ldpc)
    cfg.VHT_STBC = mtkwifi.token_set(cfg.VHT_STBC, i, iface.vht_stbc)
    cfg.HT_STBC = mtkwifi.token_set(cfg.HT_STBC, i, iface.ht_stbc)
    cfg.HT_LDPC = mtkwifi.token_set(cfg.HT_LDPC, i, iface.ht_ldpc)
    cfg.HT_PROTECT = mtkwifi.token_set(cfg.HT_PROTECT, i, iface.ht_protect)
    cfg.HT_GI = mtkwifi.token_set(cfg.HT_GI, i, iface.ht_gi)
    cfg.HT_OpMode = mtkwifi.token_set(cfg.HT_OpMode, i, iface.ht_opmode)
    cfg.HT_AMSDU = mtkwifi.token_set(cfg.HT_AMSDU, i, iface.ht_amsdu)
    cfg.HT_AutoBA = mtkwifi.token_set(cfg.HT_AutoBA, i, iface.ht_autoba)
    cfg.HT_BAWinSize = mtkwifi.token_set(cfg.HT_BAWinSize, i, iface.ht_bawinsize)
    cfg.HT_BADecline = mtkwifi.token_set(cfg.HT_BADecline, i, iface.ht_badec)
    cfg.IgmpSnEnable = mtkwifi.token_set(cfg.IgmpSnEnable, i, iface.igmpsn_enable)

    cfg.MuMimoUlEnable = mtkwifi.token_set(cfg.MuMimoUlEnable, i, iface.mumimoul_enable)
    cfg.MuMimoDlEnable = mtkwifi.token_set(cfg.MuMimoDlEnable, i, iface.mumimodl_enable)
    cfg.MuOfdmaUlEnable = mtkwifi.token_set(cfg.MuOfdmaUlEnable, i, iface.muofdmaul_enable)
    cfg.MuOfdmaDlEnable = mtkwifi.token_set(cfg.MuOfdmaDlEnable, i, iface.muofdmadl_enable)
    cfg.VOW_Group_Max_Ratio = mtkwifi.token_set(cfg.VOW_Group_Max_Ratio, i, iface.vow_group_max_ratio)
    cfg.VOW_Group_Min_Ratio = mtkwifi.token_set(cfg.VOW_Group_Min_Ratio, i, iface.vow_group_min_ratio)
    cfg.VOW_Airtime_Ctrl_En = mtkwifi.token_set(cfg.VOW_Airtime_Ctrl_En, i, iface.vow_airtime_ctrl_en)
    cfg.VOW_Group_Max_Rate = mtkwifi.token_set(cfg.VOW_Group_Max_Rate , i, iface.vow_group_max_rate)
    cfg.VOW_Group_Min_Rate = mtkwifi.token_set(cfg.VOW_Group_Min_Rate, i, iface.vow_group_min_rate)
    cfg.VOW_Rate_Ctrl_En = mtkwifi.token_set(cfg.VOW_Rate_Ctrl_En, i, iface.vow_rate_ctrl_en)

    cfg.WdsEnable = mtkwifi.token_set(cfg.WdsEnable, i, iface.wds)
    cfg.WdsList = iface.wdslist
    cfg.Wds0Key = iface.wds0key
    cfg.Wds1Key = iface.wds1key
    cfg.Wds2Key = iface.wds2key
    cfg.Wds3Key = iface.wds3key
    cfg.WdsEncrypType = iface.wdsencryptype
    cfg.WdsPhyMode = iface.wdsphymode

    local wsc_confmode, wsc_confstatus

    if iface.wps_state == '1' then
        wsc_confmode = '7'
        wsc_confstatus = '1'
    elseif iface.wps_state == '2' then
        wsc_confmode = '7'
        wsc_confstatus = '2'
    else
        wsc_confmode = '0'
        wsc_confstatus = '1'
    end

    cfg.WscConfMode = mtkwifi.token_set(cfg.WscConfMode, i, wsc_confmode)
    cfg.WscConfStatus = mtkwifi.token_set(cfg.WscConfStatus, i, wsc_confstatus)
    cfg.WscVendorPinCode = mtkwifi.token_set(cfg.WscVendorPinCode, i, iface.wps_pin or "")

    cfg["AccessPolicy"..tostring(i-1)] = iface.access_policy
    if iface.access_list ~= nil then
        for j, v in pairs(iface.access_list) do
            cfg["AccessControlList"..tostring(i-1)] = mtkwifi.token_set(cfg["AccessControlList"..tostring(i-1)], j, v)
        end
    end
    
    cfg.MultiApBacSsid = mtkwifi.token_set(cfg.MultiApBacSsid, i, iface.multi_ap_backhaul_ssid or "")
    cfg.MultiApBacPass = mtkwifi.token_set(cfg.MultiApBacPass, i, iface.multi_ap_backhaul_wpa_passphrase or "")
    cfg.AuthAlgs = mtkwifi.token_set(cfg.AuthAlgs, i, iface.auth_algs or "")
    cfg.MultiAp = mtkwifi.token_set(cfg.MultiAp, i, iface.multi_ap or "")
    cfg.IgnoreBroadSsid = mtkwifi.token_set(cfg.IgnoreBroadSsid, i, iface.ignore_broadcast_ssid or "")
    cfg.WpsCred = mtkwifi.token_set(cfg.WpsCred, i, iface.wps_cred_add_sae or "")
    cfg.WpsInde = mtkwifi.token_set(cfg.WpsInde, i, iface.wps_independent or "")
    cfg.DppPfs = mtkwifi.token_set(cfg.DppPfs, i, iface.dpp_pfs or "")
    cfg.Interwork = mtkwifi.token_set(cfg.Interwork, i, iface.interworking or "")
    cfg.SaePwe = mtkwifi.token_set(cfg.SaePwe, i, iface.sae_pwe or "")
    cfg.EHT_ApEmlsr_mr = mtkwifi.token_set(cfg.EHT_ApEmlsr_mr, i, iface.eml_mode)
    cfg.EHT_ApEmlsr_mr_trans_to = mtkwifi.token_set(cfg.EHT_ApEmlsr_mr_trans_to, i, iface.eml_trans_to)
    cfg.EHT_ApEmlsr_mr_OMN = mtkwifi.token_set(cfg.EHT_ApEmlsr_mr_OMN, i, iface.eml_omn_en)
    cfg.Mrsno_En = mtkwifi.token_set(cfg.Mrsno_En, i, iface.mrsno_enable)

    return cfg
end

local function apcli2cfg(iface, cfg)
    if tonumber(iface.disabled) == 0 then
        cfg.ApCliEnable = "1"
    else
        cfg.ApCliEnable = "0"
    end

    cfg.ApCliSsid = iface.ssid
    cfg.ApCliBssid = iface.bssid

    auth, encr = uci2dat_encryption(iface.encryption)
    cfg.ApCliAuthMode = auth
    cfg.ApCliEncrypType = encr

    if encr == "WEP" then
        cfg.ApCliDefaultKeyID = iface.key
    elseif auth == "WPA2PSK" or auth == "WPA3PSK" or auth == "AKM24" or 
        auth == "WPAPSKWPA2PSK" or auth == "WPA2PSKWPA3PSK" or auth == "WPAPSK" then
        cfg.ApCliWPAPSK = iface.key
    end

    if iface.ieee80211w == '2' then
        cfg.ApCliPMFMFPC = '1'
        cfg.ApCliPMFMFPR = '1'
    elseif iface.ieee80211w == '1' then
        cfg.ApCliPMFMFPC = '1'
        cfg.ApCliPMFMFPR = '0'
    elseif iface.ieee80211w == '0' then
        cfg.ApCliPMFMFPC = '0'
        cfg.ApCliPMFMFPR = '0'
    end

    cfg.ApCliPMFSHA256 = iface.pmf_sha256
    cfg.ApCliOWETranIe = iface.owetrante
    cfg.MACRepeaterEn = iface.mac_repeateren

    local j
    for j = 1, 4 do
        local k = iface["key"..tostring(j)] or ''
        local len = #k
        if (len == 10 or len == 26 or len == 32) and k == string.match(k, '%x+') then
            cfg["ApCliKey"..tostring(j).."Type"] = '0'
        elseif (len == 5 or len == 13 or len == 16) then
            cfg["ApCliKey"..tostring(j).."Type"] = '1'
        else
            cfg["ApCliKey"..tostring(j).."Type"] = ''
        end
        cfg["ApCliKey"..tostring(j).."Str"] = k
    end

    return cfg
end

function mldgroup2cfg(mldgroup)
    local mldObj = mldgroup
    local auth, encr = uci2dat_encryption(mldgroup.encryption)
    mldObj.name = mldgroup[".name"]
    mldObj.auth = auth
    mldObj.encr = encr
    return mldObj
end

function mtkwifi.load_cfg(dev_name, path, load_mld)
    if path == "" or path == nil then
        ucicfg = mtkwifi.uci_load_wireless(uciCfgfile)
    else
        ucicfg = mtkwifi.uci_load_wireless(path)
    end
    if not ucicfg then
        nixio.syslog("err", "unable to decode uci")
        return
    end

    local cfg = {}

    if load_mld then
        if ucicfg["wifi-mld"] ~= nil then
            cfg.mld = {}
            mld_num = 1
            for _, mldgroup in pairs(ucicfg["wifi-mld"]) do
                if mldgroup ~= nil then
                    cfg.mld[mld_num] = mldgroup2cfg(mldgroup)
                    mld_num = mld_num + 1
                end
            end
        end
    end

    for k, dev in pairs(ucicfg["wifi-device"]) do
        local devname = string.gsub(dev[".name"], "%_", ".")
        if (dev_name == devname) then
            dev2cfg(dev, cfg)

            local vifs = get_vifs_by_dev(ucicfg, dev[".name"])
            local bssid_num = 0

            for _, iface in pairs(vifs) do
                local i = string.match(iface[".name"], "%d+")
                i = tonumber(i) + 1
                iface2cfg(iface, i, cfg)
                bssid_num = bssid_num + 1
            end
            cfg.BssidNum = tostring(bssid_num)

            --apcli
            local apcli = {}
            for vifname, apcli in pairs(ucicfg["wifi-iface"]) do
                if apcli.device == dev[".name"] and apcli.mode == 'sta' then
                    apcli2cfg(apcli, cfg)
                    break
                end
            end
        end
    end

    return cfg
end

function mtkwifi.load_mapd()
    local mapd_cfg = {}
    local mapd_user = {}
    local mapd_strng = {}
    local dpp_cfg = {}
    --mapd_cfg
    mapd_cfg.DeviceRole = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.DeviceRole") or "")
    mapd_cfg.SteerEnable = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.SteerEnable") or "")
    mapd_cfg.BhPriority2G = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.BhPriority2G") or "")
    mapd_cfg.BhPriority5GH = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.BhPriority5GH") or "")
    mapd_cfg.BhPriority5GL = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.BhPriority5GL") or "")
    mapd_cfg.BhPriority6G = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.BhPriority6G") or "")
    mapd_cfg.APSteerRssiTh = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_cfg.APSteerRssiTh") or "")

    --mapd_user
    local mode = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_user.mode") or "")
    if mode ~= "" then
        mapd_user.mode = mode
    end
    mapd_user.MeshSREnable = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_user.MeshSREnable") or "")

    --mapd_strng
    mapd_strng.CUOverloadTh_2G = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_2G") or "")
    mapd_strng.CUOverloadTh_5G_L = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_5G_L") or "")
    mapd_strng.CUOverloadTh_5G_H = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_5G_H") or "")
    mapd_strng.CUOverloadTh_6G = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_6G") or "")

    --dpp_cfg
    dpp_cfg.allowed_role = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.dpp_cfg.allowed_role") or "")
    dpp_cfg.presence_band_priority = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.dpp_cfg.presence_band_priority") or "")
    if mtkwifi.read_pipe("uci get mapd.dpp_cfg.agt_qr_code") ~= "" then
        dpp_cfg.agt_qr_code = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd.dpp_cfg.agt_qr_code") or "")
    end

    return mapd_cfg, mapd_user, mapd_strng, dpp_cfg
end

function mtkwifi.save_profile(cfgs, path)

    if not cfgs then
        debug_write("configuration was empty, nothing saved")
        return
    end

    -- Keep a backup of last profile settings
    -- if string.match(path, "([^/]+)\.dat") then
       -- os.execute("cp -f "..path.." "..mtkwifi.__profile_previous_settings_path(path))
    -- end
    local datobj = datconf.openfile(path)
    datobj:merge(cfgs)
    datobj:close(true) -- means close and commit

    if pcall(require, "mtknvram") then
        local nvram = require("mtknvram")
        local l1dat, l1 = mtkwifi.__get_l1dat()
        local zone = l1 and l1.l1_path_to_zone(path)

        if pcall(require, "map_helper") and zone == "dev00" then
            mtkwifi.save_easymesh_profile_to_nvram()
        else
            if not l1dat then
                debug_write("save_profile: no l1dat", path)
                nvram.nvram_save_profile(path)
            else
                if zone then
                    debug_write("save_profile:", path, zone)
                    nvram.nvram_save_profile(path, zone)
                else
                    debug_write("save_profile:", path)
                    nvram.nvram_save_profile(path)
                end
            end
        end
    end
    os.execute("sync >dev/null 2>&1")
end

function mtkwifi.save_mesh_profile()
    os.execute("lua /etc/uci2map.lua")
    if pcall(require, "mtknvram") then
        local nvram = require("mtknvram")
        local l1dat, l1 = mtkwifi.__get_l1dat()
        local zone = l1 and l1.l1_path_to_zone(path)

        if pcall(require, "map_helper") and zone == "dev00" then
            mtkwifi.save_easymesh_profile_to_nvram()
        else
            if not l1dat then
                debug_write("save_profile: no l1dat", path)
                nvram.nvram_save_profile(path)
            else
                if zone then
                    debug_write("save_profile:", path, zone)
                    nvram.nvram_save_profile(path, zone)
                else
                    debug_write("save_profile:", path)
                    nvram.nvram_save_profile(path)
                end
            end
        end
    end
    os.execute("sync >dev/null 2>&1")
end

local uci_dev_options = {
    "type", "vendor", "percentag_enable", "txpower", "channel", "acs_skiplist", "channel_grp", "acs_alg", "autoch", "beacon_int",
    "txpreamble", "htmode", "pure_11b", "ht_coex", "band", "bw", "ht_extcha", "ht_txstream", "ht_rxstream",
    "shortslot", "ht_distkip", "bgprotect", "txburst", "region", "country", "aregion",
    "vht_bw_sig", "pktaggregate", "ht_mcs", "e2p_accessmode", "map_mode", "dbdc_mode",
    "etxbfencond", "itxbfen", "mutxrx_enable", "bss_color", "colocated_bssid",
    "twt_support", "individual_twt_support", "he_ldpc", "txop", "doth", "dfs_enable",
    "sre_enable", "powerup_enbale", "powerup_cckofdm", "powerup_ht20", "powerup_ht40", "powerup_vht20",
    "powerup_vht40", "powerup_vht80", "powerup_vht160", "vow_airtime_fairness_en",
    "ht_rdg", "whnat", "vow_bw_ctrl", "vow_ex_en", "sr_mode",
    "map_balance", "dfs_zero_wait", "dfs_dedicated_zero_wait", "dfs_zero_wait_default", "rd_region",
    "psc_acs", "qos_enable", "dabs_vendor_key_bitmap", "dabs_group_key_bitmap", "dfs_offchannel_cac"
}


local uci_iface_options = {
    "device", "network", "mode", "disabled", "ssid", "bssid", "vifidx", "hidden", "wmm", "dtim_period",
    "encryption", "key", "key1", "key2", "key3", "key4", "rekey_interval", "rekey_meth",
    "ieee8021x", "auth_server", "auth_port", "auth_secret", "ownip", "idle_timeout", "session_timeout",
    "rsn_preauth", "ieee80211w", "pmf_sha256", "wireless_mode", "tx_rate", "isolate",
    "rts", "frag", "apsd_capable", "vht_bw_signal", "vht_ldpc", "vht_stbc", "vht_sgi",
    "ht_ldpc", "ht_stbc","ht_protect", "ht_gi", "ht_opmode", "ht_amsdu", "ht_autoba", "ht_bawinsize", "ht_badec",
    "igmpsn_enable", "mumimoul_enable", "mumimodl_enable", "muofdmaul_enable", "muofdmadl_enable",
    "vow_group_max_ratio", "vow_group_min_ratio", "vow_airtime_ctrl_en", "vow_group_max_rate", "vow_group_min_rate",
    "vow_rate_ctrl_en", "pmk_cache_period", "wds", "wdslist", "wds0key", "wds1key", "wds2key", "wds3key",
    "wdsencryptype", "wdsphymode", "wps_state", "wps_pin",  "owetrante", "mac_repeateren", "access_policy", "access_list",
    "multi_ap_backhaul_ssid", "multi_ap_backhaul_wpa_passphrase", "auth_algs", "multi_ap", "ignore_broadcast_ssid", "wps_cred_add_sae",
    "wps_independent", "mbo", "dpp_pfs", "interworking", "sae_pwe", "eml_mode", "eml_trans_to", "eml_omn_en", "rrm_beacon_report",
    "bss_transtion", "mrsno_enable"
}

local uci_mld_options = {
    "disabled", "mode", "iface", "main_iface", "mld_group", "ssid", "mld_addr", "encryption", "ieee80211w", "pmf_sha256", "key", "eml_mode"
}

local function uci_encode_options(x, uci_options, cfg_type, tbl, file)
    x:set(file, tbl[".name"], cfg_type)

    for _, i in pairs(uci_options) do
        if (tbl[i] ~= nil) then
            if type(tbl[i]) == "table" and #tbl[i] ~= 0 then
                v_table = {}
                for _, v in pairs(tbl[i]) do
                    table.insert(v_table,v)
                end
                x:set(file, tbl[".name"], i, v_table)
            elseif type(tbl[i]) == "table" and #tbl[i] == 0 then
                x:delete(file, tbl[".name"], i)
            elseif tbl[i] == '' then
                x:delete(file, tbl[".name"], i)
            else
                x:set(file, tbl[".name"], i, tbl[i])
            end
        end
    end
end

local function uci_encode_dev_options(x, dev, file)
    uci_encode_options(x, uci_dev_options, "wifi-device", dev, file)
end

local function uci_encode_iface_options(x, iface, file)
    uci_encode_options(x, uci_iface_options, "wifi-iface", iface, file)
end

local function uci_encode_mld_options(x, mldgroup, file)
    uci_encode_options(x, uci_mld_options, "wifi-mld", mldgroup, file)
end

local function table_clone(org)
    local copy = {}
    for k, v in pairs(org) do
        copy[k] = v
    end
    return copy
end

local function get_drv_version(vifname)
    local driver_version = nil
    if mtkwifi.exists('/tmp/mtk/wifi/'..vifname..'_version') then
        driver_version = mtkwifi.read_pipe("cat /tmp/mtk/wifi/"..vifname.."_version")
    else
    os.execute("mwctl "..vifname.." show driverinfo > /dev/null")
    local fp = io.popen("dmesg |grep 'Driver version:'")
    local result = fp:read("*all")
    fp:close()
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        local v = line:match("Driver version:%s*(.+)")
        if  v then
            driver_version= v
                os.execute("echo "..driver_version.." > /tmp/mtk/wifi/"..vifname.."_version")
            end
        end
    end
    return driver_version
end

function cfg2mld(mld)
    local mldObj = mld
    if mld.auth==""  then
        mld.encr="NONE"
        mldObj.key=default_key
        mldObj["ieee80211w"]="2"
        mldObj["pmf_sha256"]="0"
    end
    mldObj.encryption = dat2uci_encryption(mld.auth, mld.encr)
    mldObj[".name"] = mld.name
    mldObj["eml_mode"] = mld.eml_mode or "emlsr"
    return mldObj
end

function mtkwifi.iface_type(iface)
    os.execute("iw "..iface.." info > /tmp/iface_type.txt")
    local fp = io.open("/tmp/iface_type.txt", "r")
    local num
    if not fp then return "" end
    for line in fp:lines() do
        if string.find(line, 'wiphy') then
            num = string.match(line, "%d")
        end
    end
    fp:close()

    if not num then
        return ""
    end

    return "phy"..num
end

function mtkwifi.get_band(phy)
    if not phy then
        return false
    end
    local res = io.popen("mwctl phy "..phy.." dump band_info band | grep Band | cut -d : -f 2")
    result = mtkwifi.__trim(res:read("*all"))
    res:close()
    return result
end

function mtkwifi.save_cfg(cfg, dev_name, path)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    filePath = path or uciCfgfile
    local uciCfg = mtkwifi.uci_load_wireless(filePath)
    local cfg_applied = mtkwifi.load_cfg(dev_name, mtkwifi.__uci_applied_config(), load_mld)
    local applied_intf_num = tonumber(cfg_applied.BssidNum)
    local intf_num = tonumber(cfg.BssidNum)

    local x = uci.cursor()

    local dridx = l1.DEV_RINDEX

    for _, dev in pairs(uciCfg["wifi-device"]) do
        local devname = string.gsub(dev[".name"], "%_", ".")
        if dev_name == devname then
            cfg2dev(cfg, devname, dev)
        end
        uci_encode_dev_options(x, dev, filePath)

        local i = 1
        if dev_name == devname then
            local main_ifname = l1dat[dridx][devname].ext_ifname
            if applied_intf_num > intf_num and filePath == "wireless_applied" then
                for index=1, applied_intf_num-1 do
                    os.execute("uci delete wireless_applied."..main_ifname..index)
                end
                x:commit("wireless_applied")
            end

            while i <= tonumber(cfg.BssidNum) do
                iface = {}
                ifname = main_ifname..(i-1)
                cfg2iface(cfg, dev[".name"], ifname, iface, i)
                uci_encode_iface_options(x, iface, filePath)
                i = i + 1
            end
         else
            local vifs = get_vifs_by_dev(uciCfg, dev[".name"])
            for _, iface in pairs(vifs) do
                uci_encode_iface_options(x, iface, filePath)
            end
        end

        for _, iface in pairs(uciCfg["wifi-iface"]) do
            if iface.device == dev[".name"] and iface.mode == 'sta' then
                if ( dev_name == devname ) then
                    cfg2apcli(cfg, dev[".name"], iface[".name"], iface, i)
                    --apcli2cfg(iface, cfg)
                end
                uci_encode_iface_options(x, iface, filePath)
                break
            end
        end
    end

    if cfg.mld then
        for _, mld in pairs(cfg.mld) do
            local mldObj = cfg2mld(mld)
            uci_encode_mld_options(x, mldObj, filePath)
        end
    end
    x:commit(filePath)
end

function mtkwifi.split_profile(path, path_2g, path_5g)
    assert(path)
    assert(path_2g)
    assert(path_5g)
    local cfgs = mtkwifi.load_profile(path)
    local dirty = {
        "Channel",
        "WirelessMode",
        "TxRate",
        "WmmCapable",
        "NoForwarding",
        "HideSSID",
        "IEEE8021X",
        "PreAuth",
        "AuthMode",
        "EncrypType",
        "RekeyMethod",
        "RekeyInterval",
        "PMKCachePeriod",
        "DefaultKeyId",
        "Key{n}Type",
        "HT_EXTCHA",
        "RADIUS_Server",
        "RADIUS_Port",
        "MldGroup"
    }
    local cfg5g = mtkwifi.deepcopy(cfgs)
    for _,v in ipairs(dirty) do
        cfg5g[v] = mtkwifi.token_get(cfgs[v], 1, 0)
        assert(cfg5g[v])
    end
    mtkwifi.save_profile(cfg5g, path_5g)

    local cfg2g = mtkwifi.deepcopy(cfgs)
    for _,v in ipairs(dirty) do
        cfg2g[v] = mtkwifi.token_get(cfgs[v], 1, 0)
        assert(cfg2g[v])
    end
    mtkwifi.save_profile(cfg2g, path_2g)
end

function mtkwifi.merge_profile(path, path_2g, path_5g)
    local cfg2g = mtkwifi.load_profile(path_2g)
    local cfg5g = mtkwifi.load_profile(path_5g)
    local dirty = {
        "Channel",
        "WirelessMode",
        "TxRate",
        "WmmCapable",
        "NoForwarding",
        "HideSSID",
        "IEEE8021X",
        "PreAuth",
        "AuthMode",
        "EncrypType",
        "RekeyMethod",
        "RekeyInterval",
        "PMKCachePeriod",
        "DefaultKeyId",
        "Key{n}Type",
        "HT_EXTCHA",
        "RADIUS_Server",
        "RADIUS_Port",
        "MldGroup"
    }
    local cfgs = mtkwifi.deepcopy(cfg2g)
    for _,v in dirty do
        -- TODO
    end
    mtkwifi.save_profile(cfgs, path)
end

-- update path1 by path2
function mtkwifi.update_profile(path1, path2)
    local cfg1 = datconf.openfile(path1)
    local cfg2 = datconf.openfile(path2)

    cfg1:merge(cfg2:getall())
    cfg1:close(true)
    cfg2:close()
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.__child_info_path()
    local path = "/tmp/mtk/wifi/child_info.dat"
    os.execute("mkdir -p /tmp/mtk/wifi")
    return path
end

function mtkwifi.__profile_previous_settings_path(profile)
    assert(type(profile) == "string")
    local bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.dat")..".last"
    os.execute("mkdir -p /tmp/mtk/wifi")
    return bak
end

function mtkwifi.__profile_applied_settings_path(profile)
    assert(type(profile) == "string")
    local bak
    if string.match(profile, "([^/]+)\.dat") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.dat")..".applied"
    elseif string.match(profile, "([^/]+)\.txt") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.txt")..".applied"
    elseif string.match(profile, "([^/]+)$") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)$")..".applied"
    else
        bak = ""
    end
    return bak
end

function mtkwifi.__uci_applied_settings_path()
    local bak = "/etc/config/wireless_applied"
    return bak
end

function mtkwifi.__uci_applied_config()
    local bak = "wireless_applied"
    return bak
end

-- if path2 is not given, use backup of path1.
function mtkwifi.diff_profile(path1, path2)
    assert(path1)
    if not path2 then
        path2 = mtkwifi.__profile_applied_settings_path(path1)
        if not mtkwifi.exists(path2) then
            return {}
        end
    end
    assert(path2)

    local cfg1
    local cfg2
    local diff = {}
    if path1 == mtkwifi.__easymesh_bss_cfgs_path() then
        cfg1 = mtkwifi.get_file_lines(path1) or {}
        cfg2 = mtkwifi.get_file_lines(path2) or {}
    else
        cfg1 = mtkwifi.load_profile(path1) or {}
        cfg2 = mtkwifi.load_profile(path2) or {}
    end

    for k,v in pairs(cfg1) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end

    for k,v in pairs(cfg2) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end

    return diff
end

function mtkwifi.diff_cfg(dev, load_mld)
    assert(dev)
    local path = mtkwifi.__uci_applied_settings_path()

    if not mtkwifi.exists(path) then
        return {}
    end

    local cfg1
    local cfg2
    local diff = {}

    cfg1 = mtkwifi.load_cfg(dev, "wireless", load_mld) or {}
    cfg2 = mtkwifi.load_cfg(dev, mtkwifi.__uci_applied_config(), load_mld) or {}

    for k,v in pairs(cfg1) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end

    for k,v in pairs(cfg2) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end
    return diff
end

function mtkwifi.diff_mld()
    local path = "/etc/config/wireless"
    if not mtkwifi.exists(mtkwifi.__uci_applied_settings_path()) then
        return false
    end
    local cfg1, cfg2
    cfg1 = mtkwifi.load_cfg("", "" ,true)
    cfg2 = mtkwifi.load_cfg("", mtkwifi.__uci_applied_config() ,true)
    if not cfg1.mld then cfg1.mld = {} end
    if not cfg2.mld then cfg2.mld = {} end

    if #cfg1["mld"] ~= #cfg2["mld"] then
        return true
    end

    if #cfg1["mld"] == 0 and #cfg2["mld"] ==0 then
        return false
    end

    for i,j in pairs(cfg1.mld) do
        for k,v in pairs(j) do
            if j[k] ~= cfg2.mld[i][k] then
                return true
            end
        end
    end

    for i,j in pairs(cfg2.mld) do
        for k,v in pairs(j) do
            if j[k] ~= cfg1.mld[i][k] then
                return true
            end
        end
    end
end

function mtkwifi.__fork_exec(command)
    if type(command) ~= type("") or command == "" then
        debug_write("__fork_exec : Incorrect command! Expected non-empty string type, got ",type(command))
        nixio.syslog("err", "__fork_exec : Incorrect command! Expected non-empty string type, got "..type(command))
    else
        local nixio = require("nixio")
        -- If nixio.exec() fails, then child process will be reaped automatically and
        -- it will be achieved by ignoring SIGCHLD signal here in parent process!
        if not nixio.signal(17,"ign") then
            nixio.syslog("warning", "__fork_exec : Failed to set SIG_IGN for SIGCHLD!")
            debug_write("__fork_exec : Failed to set SIG_IGN for SIGCHLD!")
        end
        local pid = nixio.fork()
        if pid < 0 then
            nixio.syslog("err", "__fork_exec : [Fork Failure] "..command)
            debug_write("__fork_exec : [Fork Failure] "..command)
        elseif pid == 0 then
            -- change to root dir to flush out any opened directory streams of parent process.
            nixio.chdir("/")

            -- As file descriptors are inherited by child process, all unused file descriptors must be closed.
            -- Make stdin, out, err file descriptors point to /dev/null using dup2.
            -- As a result, it will not corrupt stdin, out, err file descriptors of parent process.
            local null = nixio.open("/dev/null", "w+")
            if null then
                nixio.dup(null, nixio.stderr)
                nixio.dup(null, nixio.stdout)
                nixio.dup(null, nixio.stdin)
                if null:fileno() > 2 then
                    null:close()
                end
            end
            debug_write("__fork_exec : cmd = "..command)
            -- replaces the child process image with the new process image generated by provided command
            nixio.exec("/bin/sh", "-c", command)
            os.exit(true)
        end
    end
end

function mtkwifi.is_child_active()
    local fd = io.open(mtkwifi.__child_info_path(), "r")
    if not fd then
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    local content = fd:read("*all")
    fd:close()
    if not content then
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    local active_pid_list = {}
    for _,pid in ipairs(mtkwifi.__lines(content)) do
        pid = pid:match("CHILD_PID=%s*(%d+)%s*")
        if pid then
            if tonumber(mtkwifi.read_pipe("ps | grep -v grep | grep -cw "..pid)) == 1 then
                table.insert(active_pid_list, pid)
            end
        end
    end
    if next(active_pid_list) ~= nil then
        return true
    else
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.__run_in_child_env(cbFn,...)
    if type(cbFn) ~= "function" then
        debug_write("__run_in_child_env : Function type expected, got ", type(cbFn))
        nixio.syslog("err", "__run_in_child_env : Function type expected, got "..type(cbFn))
    else
        local unpack = unpack or table.unpack
        local cbArgs = {...}
        local nixio = require("nixio")
        -- Let child process reap automatically!
        if not nixio.signal(17,"ign") then
            nixio.syslog("warning", "__run_in_child_env : Failed to set SIG_IGN for SIGCHLD!")
            debug_write("__run_in_child_env : Failed to set SIG_IGN for SIGCHLD!")
        end
        local pid = nixio.fork()
        if pid < 0 then
            debug_write("__run_in_child_env : Fork failure")
            nixio.syslog("err", "__run_in_child_env : Fork failure")
        elseif pid == 0 then
            -- Change to root dir to flush out any opened directory streams of parent process.
            nixio.chdir("/")

            -- As file descriptors are inherited by child process, all unnecessary file descriptors must be closed.
            -- Make stdin, out, err file descriptors point to /dev/null using dup2.
            -- As a result, it will not corrupt stdin, out, err file descriptors of parent process.
            local null = nixio.open("/dev/null", "w+")
            if null then
                nixio.dup(null, nixio.stderr)
                nixio.dup(null, nixio.stdout)
                nixio.dup(null, nixio.stdin)
                if null:fileno() > 2 then
                    null:close()
                end
            end
            local fd = io.open(mtkwifi.__child_info_path(), "a")
            if fd then
                fd:write("CHILD_PID=",nixio.getpid(),"\n")
                fd:close()
            end
            cbFn(unpack(cbArgs))

            os.exit(true)
        end
    end
    os.execute("sync >/dev/null 2>&1")
end

-- Mode 12 and 13 are only available for STAs.
local WirelessModeList = {
    [0] = "B/G mixed",
    [1] = "B only",
    [2] = "A only",
    -- [3] = "A/B/G mixed",
    [4] = "G only",
    -- [5] = "A/B/G/GN/AN mixed",
    [6] = "N in 2.4G only",
    [7] = "G/GN", -- i.e., no CCK mode
    [8] = "A/N in 5 band",
    [9] = "B/G/GN mode",
    -- [10] = "A/AN/G/GN mode", --not support B mode
    [11] = "only N in 5G band",
    -- [12] = "B/G/GN/A/AN/AC mixed",
    -- [13] = "G/GN/A/AN/AC mixed", -- no B mode
    [14] = "A/AC/AN mixed",
    [15] = "AC/AN mixed", --but no A mode
    [16] = "HE_2G mode", --HE Wireless Mode
    [17] = "HE_5G mode", --HE Wireless Mode
    [18] = "HE_6G mode", --HE Wireless Mode
    [22] = "BE_24G mode", --BE Wireless Mode
    [23] = "BE_5G mode", --BE Wireless Mode
    [24] = "BE_6G mode" --BE Wireless Mode
}

local DevicePropertyMap = {
    -- 2.4G
    {
        device="MT7622",
        band={"0", "1", "4", "9"},
        isPowerBoostSupported=true,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7620",
        band={"0", "1", "4", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8
    },

    {
        device="MT7628",
        band={"0", "1", "4", "6", "7", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7603",
        band={"0", "1", "4", "6", "7", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    -- 5G
    {
        device="MT7612",
        band={"2", "8", "11", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7662",
        band={"2", "8", "11", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
    },

    -- Mix
    {
        device="MT7615",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=8
    },

    {
        device="MT7915",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161}
    },

    {
        device="MT7916",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7981",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7986",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },

    {
        device="MT7902",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18","22","23","24"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },
    {
        device="MT7990",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18","22","23","24"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },
    {
        device="MT7992",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18","22","23","24"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },
    {
        device="MT7993",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18","22","23","24"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },
    {
        device="MT7663",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
        invalidChBwList={160,161},
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7613",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
        invalidChBwList={160,161},
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7626",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=3,
        maxRxStream=3,
        invalidChBwList={160,161},
        wdsBand="2.4G",
        mimoBand="5G",
        maxDBDCVif=8
    },

    {
        device="MT7629",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=3,
        maxRxStream=3,
        invalidChBwList={160,161},
        wdsBand="2.4G",
        mimoBand="5G",
        maxDBDCVif=8,
        isMultiAPSupported=true
    }
}

mtkwifi.CountryRegionList_6G_All = {
    {region=0, text="0: Ch1~233"},
    {region=1, text="1: Ch1~93"},
    {region=2, text="2: Ch97~113"},
    {region=3, text="3: Ch117~185"},
    {region=4, text="4: Ch189~233"},
    {region=5, text="5: Ch1~93"},
    {region=6, text="6: Ch1~93"},
    {region=7, text="7: Ch1~93, Ch97~113"},
}

mtkwifi.CountryRegionList_5G_All = {
    {region=0, text="0: Ch36~64, Ch149~165"},
    {region=1, text="1: Ch36~64, Ch100~140"},
    {region=2, text="2: Ch36~64"},
    {region=3, text="3: Ch52~64, Ch149~161"},
    {region=4, text="4: Ch149~165"},
    {region=5, text="5: Ch149~161"},
    {region=6, text="6: Ch36~48"},
    {region=7, text="7: Ch36~64, Ch100~140, Ch149~165"},
    {region=8, text="8: Ch52~64"},
    {region=9, text="9: Ch36~64, Ch100~116, Ch132~140, Ch149~165"},
    {region=10, text="10: Ch36~48, Ch149~165"},
    {region=11, text="11: Ch36~64, Ch100~120, Ch149~161"},
    {region=12, text="12: Ch36~64, Ch100~144"},
    {region=13, text="13: Ch36~64, Ch100~144, Ch149~165"},
    {region=14, text="14: Ch36~64, Ch100~116, Ch132~144, Ch149~165"},
    {region=15, text="15: Ch149~173"},
    {region=16, text="16: Ch52~64, Ch149~165"},
    {region=17, text="17: Ch36~48, Ch149~161"},
    {region=18, text="18: Ch36~64, Ch100~116, Ch132~140"},
    {region=19, text="19: Ch56~64, Ch100~140, Ch149~161"},
    {region=20, text="20: Ch36~64, Ch100~124, Ch149~161"},
    {region=21, text="21: Ch36~64, Ch100~140, Ch149~161"},
    {region=22, text="22: Ch100~140"},
    {region=23, text="23: ch36~64, ch100~116, ch132~144"},
    {region=24, text="24: ch100~144"},
    {region=25, text="25: ch36~64, ch100~116, ch132~140, ch149~177"},
    {region=26, text="26: ch36~64, ch100~144, ch149~177"},
    -- {region=30, text="30: Ch36~48, Ch52~64, Ch100~140, Ch149~165"},
    -- {region=31, text="31: Ch52~64, Ch100~140, Ch149~165"},
    -- {region=32, text="32: Ch36~48, Ch52~64, Ch100~140, Ch149~161"},
    -- {region=33, text="33: Ch36~48, Ch52~64, Ch100~140"},
    -- {region=34, text="34: Ch36~48, Ch52~64, Ch149~165"},
    -- {region=35, text="35: Ch36~48, Ch52~64"},
    -- {region=36, text="36: Ch36~48, Ch100~140, Ch149~165"},
    -- {region=37, text="37: Ch36~48, Ch52~64, Ch149~165, Ch173"}
}

mtkwifi.CountryRegionList_2G_All = {
    {region=0, text="0: Ch1~11"},
    {region=1, text="1: Ch1~13"},
    {region=2, text="2: Ch10~11"},
    {region=3, text="3: Ch10~13"},
    {region=4, text="4: Ch14"},
    {region=5, text="5: Ch1~14"},
    {region=6, text="6: Ch3~9"},
    {region=7, text="7: Ch5~13"},
    {region=31, text="31: Ch1~11, Ch12~14"},
    {region=32, text="32: Ch1~11, Ch12~13"},
    {region=33, text="33: Ch1~14"}
}

mtkwifi.ChannelList_6G_All = {
    {channel= 0  , text="Channel 0 (Auto )", region={}},
    {channel= 1  , text="Channel  1   (5.955 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 5  , text="Channel  5   (5.975 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 9  , text="Channel  9   (5.995 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 13 , text="Channel  13  (6.015 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 17 , text="Channel  17  (6.035 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 21 , text="Channel  21  (6.055 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 25 , text="Channel  25  (6.075 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 29 , text="Channel  29  (6.095 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 33 , text="Channel  33  (6.115 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 37 , text="Channel  37  (6.135 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 41 , text="Channel  41  (6.155 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 45 , text="Channel  45  (6.175 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 49 , text="Channel  49  (6.195 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 53 , text="Channel  53  (6.215 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 57 , text="Channel  57  (6.235 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 61 , text="Channel  61  (6.255 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 65 , text="Channel  65  (6.275 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 69 , text="Channel  69  (6.295 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 73 , text="Channel  73  (6.315 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 77 , text="Channel  77  (6.335 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 81 , text="Channel  81  (6.355 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 85 , text="Channel  85  (6.375 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 89 , text="Channel  89  (6.395 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 93 , text="Channel  93  (6.415 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 97 , text="Channel  97  (6.435 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 101, text="Channel  101 (6.455 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 105, text="Channel  105 (6.475 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 109, text="Channel  109 (6.495 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 113, text="Channel  113 (6.515 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 117, text="Channel  117 (6.535 GHz)", region={[0]=1, [3]=1}},
    {channel= 121, text="Channel  121 (6.555 GHz)", region={[0]=1, [3]=1}},
    {channel= 125, text="Channel  125 (6.575 GHz)", region={[0]=1, [3]=1}},
    {channel= 129, text="Channel  129 (6.595 GHz)", region={[0]=1, [3]=1}},
    {channel= 133, text="Channel  133 (6.615 GHz)", region={[0]=1, [3]=1}},
    {channel= 137, text="Channel  137 (6.635 GHz)", region={[0]=1, [3]=1}},
    {channel= 141, text="Channel  141 (6.655 GHz)", region={[0]=1, [3]=1}},
    {channel= 145, text="Channel  145 (6.675 GHz)", region={[0]=1, [3]=1}},
    {channel= 149, text="Channel  149 (6.695 GHz)", region={[0]=1, [3]=1}},
    {channel= 153, text="Channel  153 (6.715 GHz)", region={[0]=1, [3]=1}},
    {channel= 157, text="Channel  157 (6.735 GHz)", region={[0]=1, [3]=1}},
    {channel= 161, text="Channel  161 (6.755 GHz)", region={[0]=1, [3]=1}},
    {channel= 165, text="Channel  165 (6.775 GHz)", region={[0]=1, [3]=1}},
    {channel= 169, text="Channel  169 (6.795 GHz)", region={[0]=1, [3]=1}},
    {channel= 173, text="Channel  173 (6.815 GHz)", region={[0]=1, [3]=1}},
    {channel= 177, text="Channel  177 (6.835 GHz)", region={[0]=1, [3]=1}},
    {channel= 181, text="Channel  181 (6.855 GHz)", region={[0]=1, [3]=1}},
    {channel= 185, text="Channel  185 (6.875 GHz)", region={[0]=1, [3]=1}},
    {channel= 189, text="Channel  189 (6.895 GHz)", region={[0]=1, [4]=1}},
    {channel= 193, text="Channel  193 (6.915 GHz)", region={[0]=1, [4]=1}},
    {channel= 197, text="Channel  197 (6.935 GHz)", region={[0]=1, [4]=1}},
    {channel= 201, text="Channel  201 (6.955 GHz)", region={[0]=1, [4]=1}},
    {channel= 205, text="Channel  205 (6.975 GHz)", region={[0]=1, [4]=1}},
    {channel= 209, text="Channel  209 (6.995 GHz)", region={[0]=1, [4]=1}},
    {channel= 213, text="Channel  213 (7.015 GHz)", region={[0]=1, [4]=1}},
    {channel= 217, text="Channel  217 (7.035 GHz)", region={[0]=1, [4]=1}},
    {channel= 221, text="Channel  221 (7.055 GHz)", region={[0]=1, [4]=1}},
    {channel= 225, text="Channel  225 (7.075 GHz)", region={[0]=1, [4]=1}},
    {channel= 229, text="Channel  229 (7.095 GHz)", region={[0]=1, [4]=1}},
    {channel= 233, text="Channel  233 (7.115 GHz)", region={[0]=1, [4]=1}},
}

mtkwifi.ChannelList_5G_All = {
    {channel=0,  text="Channel 0 (Auto )", region={}},
    {channel= 36, text="Channel  36 (5.180 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [23]=1, [25]=1, [26]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 40, text="Channel  40 (5.200 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [23]=1, [25]=1, [26]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 44, text="Channel  44 (5.220 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [23]=1, [25]=1, [26]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 48, text="Channel  48 (5.240 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [23]=1, [25]=1, [26]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 52, text="Channel  52 (5.260 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [20]=1, [21]=1, [23]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 56, text="Channel  56 (5.280 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [23]=1, [25]=1, [26]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 60, text="Channel  60 (5.300 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [23]=1, [25]=1, [26]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 64, text="Channel  64 (5.320 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [23]=1, [25]=1, [26]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel=100, text="Channel 100 (5.500 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=104, text="Channel 104 (5.520 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=108, text="Channel 108 (5.540 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=112, text="Channel 112 (5.560 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=116, text="Channel 116 (5.580 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=120, text="Channel 120 (5.600 GHz)", region={[1]=1, [7]=1, [11]=1, [12]=1, [13]=1, [19]=1, [20]=1, [21]=1, [22]=1, [24]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=124, text="Channel 124 (5.620 GHz)", region={[1]=1, [7]=1, [12]=1, [13]=1, [19]=1, [20]=1, [21]=1, [22]=1, [24]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=128, text="Channel 128 (5.640 GHz)", region={[1]=1, [7]=1, [12]=1, [13]=1, [19]=1, [21]=1, [22]=1, [30]=1, [24]=1, [26]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=132, text="Channel 132 (5.660 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=136, text="Channel 136 (5.680 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=140, text="Channel 140 (5.700 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [23]=1, [24]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=144, text="Channel 144 (5.720 GHz)", region={[12]=1, [13]=1, [14]=1, [23]=1, [24]=1, [26]=1}},
    {channel=149, text="Channel 149 (5.745 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=153, text="Channel 153 (5.765 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=157, text="Channel 157 (5.785 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=161, text="Channel 161 (5.805 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [25]=1, [26]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=165, text="Channel 165 (5.825 GHz)", region={[0]=1, [4]=1, [7]=1, [9]=1, [10]=1, [13]=1, [14]=1, [15]=1, [16]=1, [25]=1, [26]=1, [30]=1, [31]=1, [34]=1, [36]=1, [37]=1}},
    {channel=169, text="Channel 169 (5.845 GHz)", region={[15]=1, [25]=1, [26]=1}},
    {channel=173, text="Channel 173 (5.865 GHz)", region={[15]=1, [25]=1, [26]=1, [37]=1}},
    {channel=177, text="Channel 177 (5.885 GHz)", region={[25]=1, [26]=1}}
}

mtkwifi.ChannelList_2G_All = {
    {channel=0, text="Channel 0 (Auto )", region={}},
    {channel= 1, text="Channel  1 (2.412 GHz)", region={[0]=1, [1]=1, [5]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 2, text="Channel  2 (2.417 GHz)", region={[0]=1, [1]=1, [5]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 3, text="Channel  3 (2.422 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 4, text="Channel  4 (2.427 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 5, text="Channel  5 (2.432 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 6, text="Channel  6 (2.437 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 7, text="Channel  7 (2.442 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 8, text="Channel  8 (2.447 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 9, text="Channel  9 (2.452 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=10, text="Channel 10 (2.457 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=11, text="Channel 11 (2.462 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=12, text="Channel 12 (2.467 GHz)", region={[1]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=13, text="Channel 13 (2.472 GHz)", region={[1]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=14, text="Channel 14 (2.477 GHz)", region={[4]=1, [5]=1, [31]=1, [33]=1}}
}

mtkwifi.ChannelList_5G_2nd_80MHZ_ALL = {
    {channel=36, text="Ch36(5.180 GHz) - Ch48(5.240 GHz)", chidx=2},
    {channel=52, text="Ch52(5.260 GHz) - Ch64(5.320 GHz)", chidx=6},
    {channel=-1, text="Channel between 64 100",  chidx=-1},
    {channel=100, text="Ch100(5.500 GHz) - Ch112(5.560 GHz)", chidx=10},
    {channel=112, text="Ch116(5.580 GHz) - Ch128(5.640 GHz)", chidx=14},
    {channel=-1, text="Channel between 128 132", chidx=-1},
    {channel=132, text="Ch132(5.660 GHz) - Ch144(5.720 GHz)", chidx=18},
    {channel=-1, text="Channel between 144 149", chidx=-1},
    {channel=149, text="Ch149(5.745 GHz) - Ch161(5.805 GHz)", chidx=22}
}

local AuthModeList = {
    "Disable",
    -- "OPEN",
    "Enhanced Open",
    -- "SHARED",
    -- "WEPAUTO",
    "WPA2",
    "WPA3",
    "WPA3-192-bit",
    "WPA2PSK",
    "WPA3PSK",
    "WPAPSKWPA2PSK",
    "WPA2PSKWPA3PSK",
    "WPA1WPA2",
    "IEEE8021X"
}

local AuthModeList_6G = {
    "Enhanced Open",
    "WPA3",
    "WPA3PSK",
    --"WPA2PSKWPA3PSK"
}
local WpsEnableAuthModeList = {
    "Disable",
    -- "OPEN",
    "WPA2PSK",
    "WPAPSKWPA2PSK"
}
local WpsEnableAuthModeList_6G = {

}
local ApCliAuthModeList = {
    "Disable",
    "Enhanced Open",
    "WPAPSK",
    "WPA2PSK",
    "WPA3PSK",
    "AKM24"
}

local ApCliAuthModeList_6G = {
    "Enhanced Open",
    "WPA3PSK",
    "AKM24"
}

local EncryptionTypeList = {
    "WEP",
    "TKIP",
    "TKIPAES",
    "AES",
    "GCMP256"
}
local EncryptionTypeList_6G = {
    "WEP",
    "AES",
    "GCMP256"
}

local dbdc_prefix = {
    {"ra",  "rax"},
    {"rai", "ray"},
    {"rae", "raz"}
}

local dbdc_apcli_prefix = {
    {"apcli",  "apclix"},
    {"apclii", "apcliy"},
    {"apclie", "apcliz"}
}

function mtkwifi.get_channel_list_from_cmd(phy)
    local ch_list = {}
    local res = io.popen("iw phy "..phy.." channels | grep 'MHz \\[' | grep -v disabled;")
    result = mtkwifi.__trim(res:read("*all"))

    for i, line in ipairs(mtkwifi.__lines(result)) do
        if mtkwifi.__trim(line) ~= "" then
            local channel_list = {}
            channel = mtkwifi.__split(mtkwifi.__trim(line), " ")[4]
            if channel then
                channel = string.match(channel, "%[(%d+)%]")
                channel_list["channel"] = channel
            end

            frequency = mtkwifi.__split(mtkwifi.__trim(line), " ")[2].." "..mtkwifi.__split(mtkwifi.__trim(line), " ")[3]
            channel_list["text"] = "Channel "..channel.." ("..frequency..")"

            table.insert(ch_list, channel_list)
        end
    end
    res:close()
    return ch_list
end

function mtkwifi.band(mode)
    local i = tonumber(mode)
    if i == 0
    or i == 1
    or i == 4
    or i == 6
    or i == 7
    or i == 9
    or i == 16
    or i == 22 then
        return "2.4G"
    elseif i == 18 or i == 24 then
        return "6G"
    else
        return "5G"
    end
end

function mtkwifi.search_dev_and_profile_orig()
    local nixio = require("nixio")
    local dir = io.popen("ls /etc/wireless/")
    if not dir then return end
    local result = {}
    -- case 1: mt76xx.dat (best)
    -- case 2: mt76xx.n.dat (multiple card of same dev)
    -- case 3: mt76xx.n.nG.dat (case 2 plus dbdc and multi-profile, bloody hell....)
    for line in dir:lines() do
        -- nixio.syslog("debug", "scan "..line)
        local tmp = io.popen("find /etc/wireless/"..line.." -type f -name \"*.dat\"")
        for datfile in tmp:lines() do
            -- nixio.syslog("debug", "test "..datfile)

            repeat do
            -- for case 1
            local devname = string.match(datfile, "("..line..").dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            -- for case 2
            local devname = string.match(datfile, "("..line.."%.%d)%.dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            -- for case 3
            local devname = string.match(datfile, "("..line.."%.%d%.%dG)%.dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            end until true
        end
    end

    for k,v in pairs(result) do
        nixio.syslog("debug", "search_dev_and_profile_orig: "..k.."="..v)
    end

    return result
end

function mtkwifi.search_dev_and_profile_l1()
    local l1dat = mtkwifi.__get_l1dat()

    if not l1dat then return end

    local nixio = require("nixio")
    local result = {}
    local dbdc_2nd_if = ""

    for k, dev in ipairs(l1dat) do
        dbdc_2nd_if = mtkwifi.token_get(dev.main_ifname, 2, nil)
        if dbdc_2nd_if then
            result[dev["INDEX"].."."..dev["mainidx"]..".1"] = mtkwifi.token_get(dev.profile_path, 1, nil)
            result[dev["INDEX"].."."..dev["mainidx"]..".2"] = mtkwifi.token_get(dev.profile_path, 2, nil)
        elseif dev["subidx"] then
            result[dev["INDEX"].."."..dev["mainidx"].."."..dev["subidx"]] = dev.profile_path
        else
            result[dev["INDEX"].."."..dev["mainidx"]] = dev.profile_path
        end
    end

    for k,v in pairs(result) do
        nixio.syslog("debug", "search_dev_and_profile_l1: "..k.."="..v)
    end

    return result
end

function mtkwifi.search_dev_and_profile()
    return mtkwifi.search_dev_and_profile_l1() or mtkwifi.search_dev_and_profile_orig()
end

function mtkwifi.__setup_vifs(cfgs, devname, mainidx, subidx)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local prefix
    local main_ifname
    local vifs = {}
    local dev_idx = ""


    prefix = l1dat and l1dat[dridx][devname].ext_ifname or dbdc_prefix[mainidx][subidx]

    dev_idx = string.match(devname, "(%w+)")

    vifs["__prefix"] = prefix
    if (cfgs.BssidNum == nil) then
        debug_write("BssidNum configuration value not found.")
        nixio.syslog("debug","BssidNum configuration value not found.")
        return
    end

    for j=1,tonumber(cfgs.BssidNum) do
        vifs[j] = {}
        vifs[j].vifidx = j -- start from 1
        dev_idx = string.match(devname, "(%w+)")
        main_ifname = l1dat and l1dat[dridx][devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
        vifs[j].vifname = j == 1 and main_ifname or prefix..(j-1)
        if mtkwifi.exists("/sys/class/net/"..vifs[j].vifname) then
            local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..vifs[j].vifname.."/flags 2>/dev/null")) or 0
            vifs[j].state = flags%2 == 1 and "up" or "down"
        end
        vifs[j].__ssid = cfgs["SSID"..j]
        local rd_pipe_output = mtkwifi.read_pipe("cat /sys/class/net/"..prefix..(j-1).."/address 2>/dev/null")
        vifs[j].__bssid = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "?"

        vifs[j].__temp_ssid = mtkwifi.__trim(mtkwifi.read_pipe("iwconfig "..vifs[j].vifname.." | grep ESSID | cut -d : -f 2-"))
        vifs[j].__temp_channel = mtkwifi.read_pipe("iwconfig "..vifs[j].vifname.." | grep Channel | cut -d = -f 2 | cut -d \" \" -f 1")
        if string.gsub(vifs[j].__temp_channel, "^%s*(.-)%s*$", "%1") == "" then
            vifs[j].__temp_channel = mtkwifi.read_pipe("iwconfig "..vifs[j].vifname.." | grep Channel | cut -d : -f 3 | cut -d \" \" -f 1")
        end

        if (vifs[j].__temp_ssid ~= "") then
            vifs[j].__ssid = vifs[j].__temp_ssid:gsub("^\"(.-)\"$","%1")
        else
            vifs[j].__ssid = cfgs["SSID"..j]
        end
        if vifs[j].__ssid ~= nil then
        vifs[j].__ssid = vifs[j].__ssid:gsub("<", "&lt")
        vifs[j].__ssid = vifs[j].__ssid:gsub(">", "&gt")
        end
        if (vifs[j].__temp_channel ~= "" ) then
            vifs[j].__channel = vifs[j].__temp_channel
        else
            vifs[j].__channel = cfgs.Channel
        end

        vifs[j].__wirelessmode = mtkwifi.token_get(cfgs.WirelessMode, j, mtkwifi.__split(cfgs.WirelessMode,";")[1])
        vifs[j].__authmode = mtkwifi.token_get(cfgs.AuthMode, j, mtkwifi.__split(cfgs.AuthMode,";")[1])
        vifs[j].__encrypttype = mtkwifi.token_get(cfgs.EncrypType, j, mtkwifi.__split(cfgs.EncrypType,";")[1])
        vifs[j].__hidessid = mtkwifi.token_get(cfgs.HideSSID, j, 0)
        vifs[j].__noforwarding = mtkwifi.token_get(cfgs.NoForwarding, j, mtkwifi.__split(cfgs.NoForwarding,";")[1])
        vifs[j].__wmmcapable = mtkwifi.token_get(cfgs.WmmCapable, j, mtkwifi.__split(cfgs.WmmCapable,";")[1])
        vifs[j].__txrate = mtkwifi.token_get(cfgs.TxRate, j, mtkwifi.__split(cfgs.TxRate,";")[1])
        vifs[j].__ieee8021x = mtkwifi.token_get(cfgs.IEEE8021X, j, mtkwifi.__split(cfgs.IEEE8021X,";")[1])
        vifs[j].__preauth = mtkwifi.token_get(cfgs.PreAuth, j, mtkwifi.__split(cfgs.PreAuth,";")[1])
        vifs[j].__rekeymethod = mtkwifi.token_get(cfgs.RekeyMethod, j, mtkwifi.__split(cfgs.RekeyMethod,";")[1])
        vifs[j].__rekeyinterval = mtkwifi.token_get(cfgs.RekeyInterval, j, mtkwifi.__split(cfgs.RekeyInterval,";")[1])
        vifs[j].__pmkcacheperiod = mtkwifi.token_get(cfgs.PMKCachePeriod, j, mtkwifi.__split(cfgs.PMKCachePeriod,";")[1])
        vifs[j].__ht_extcha = mtkwifi.token_get(cfgs.HT_EXTCHA, j, mtkwifi.__split(cfgs.HT_EXTCHA,";")[1])
        vifs[j].__radius_server = mtkwifi.token_get(cfgs.RADIUS_Server, j, mtkwifi.__split(cfgs.RADIUS_Server,";")[1])
        vifs[j].__radius_port = mtkwifi.token_get(cfgs.RADIUS_Port, j, mtkwifi.__split(cfgs.RADIUS_Port,";")[1])
        vifs[j].__wepkey_id = mtkwifi.token_get(cfgs.DefaultKeyID, j, mtkwifi.__split(cfgs.DefaultKeyID,";")[1])
        vifs[j].__wscconfmode = mtkwifi.token_get(cfgs.WscConfMode, j, mtkwifi.__split(cfgs.WscConfMode,";")[1])
        vifs[j].__wepkeys = {
            cfgs["Key1Str"..j],
            cfgs["Key2Str"..j],
            cfgs["Key3Str"..j],
            cfgs["Key4Str"..j],
        }
        vifs[j].__wpapsk = cfgs["WPAPSK"..j]
        vifs[j].__ht_stbc = mtkwifi.token_get(cfgs.HT_STBC, j, mtkwifi.__split(cfgs.HT_STBC,";")[1])
        vifs[j].__ht_ldpc = mtkwifi.token_get(cfgs.HT_LDPC, j, mtkwifi.__split(cfgs.HT_LDPC,";")[1])
        vifs[j].__vht_stbc = mtkwifi.token_get(cfgs.VHT_STBC, j, mtkwifi.__split(cfgs.VHT_STBC,";")[1])
        vifs[j].__vht_ldpc = mtkwifi.token_get(cfgs.VHT_LDPC, j, mtkwifi.__split(cfgs.VHT_LDPC,";")[1])
        --vifs[j].__dls_capable = mtkwifi.token_get(cfgs.DLSCapable, j, mtkwifi.__split(cfgs.DLSCapable,";")[1])
        vifs[j].__apsd_capable = mtkwifi.token_get(cfgs.APSDCapable, j, mtkwifi.__split(cfgs.APSDCapable,";")[1])
        vifs[j].__frag_threshold = mtkwifi.token_get(cfgs.FragThreshold, j, mtkwifi.__split(cfgs.FragThreshold,";")[1])
        vifs[j].__rts_threshold = mtkwifi.token_get(cfgs.RTSThreshold, j, mtkwifi.__split(cfgs.RTSThreshold,";")[1])
        vifs[j].__vht_sgi = mtkwifi.token_get(cfgs.VHT_SGI, j, mtkwifi.__split(cfgs.VHT_SGI,";")[1])
        vifs[j].__vht_bw_signal = mtkwifi.token_get(cfgs.VHT_BW_SIGNAL, j, mtkwifi.__split(cfgs.VHT_BW_SIGNAL,";")[1])
        vifs[j].__ht_protect = mtkwifi.token_get(cfgs.HT_PROTECT, j, mtkwifi.__split(cfgs.HT_PROTECT,";")[1])
        vifs[j].__ht_gi = mtkwifi.token_get(cfgs.HT_GI, j, mtkwifi.__split(cfgs.HT_GI,";")[1])
        vifs[j].__ht_opmode = mtkwifi.token_get(cfgs.HT_OpMode, j, mtkwifi.__split(cfgs.HT_OpMode,";")[1])
        vifs[j].__ht_amsdu = mtkwifi.token_get(cfgs.HT_AMSDU, j, mtkwifi.__split(cfgs.HT_AMSDU,";")[1])
        vifs[j].__ht_autoba = mtkwifi.token_get(cfgs.HT_AutoBA, j, mtkwifi.__split(cfgs.HT_AutoBA,";")[1])
        vifs[j].__ht_bawinsize = mtkwifi.token_get(cfgs.HT_BAWinSize, j, mtkwifi.__split(cfgs.HT_BAWinSize,";")[1])
        vifs[j].__ht_badecline = mtkwifi.token_get(cfgs.HT_BADecline, j, mtkwifi.__split(cfgs.HT_BADecline,";")[1])
        vifs[j].__igmp_snenable = mtkwifi.token_get(cfgs.IgmpSnEnable, j, mtkwifi.__split(cfgs.IgmpSnEnable,";")[1])
        vifs[j].__wdsenable = mtkwifi.token_get(cfgs.WdsEnable, j, mtkwifi.__split(cfgs.WdsEnable,";")[1])

        -- VoW
        vifs[j].__atc_tp     = mtkwifi.token_get(cfgs.VOW_Rate_Ctrl_En,    j, mtkwifi.__split(cfgs.VOW_Rate_Ctrl_En,";")[1])
        vifs[j].__atc_min_tp = mtkwifi.token_get(cfgs.VOW_Group_Min_Rate,  j, mtkwifi.__split(cfgs.VOW_Group_Min_Rate,";")[1])
        vifs[j].__atc_max_tp = mtkwifi.token_get(cfgs.VOW_Group_Max_Rate,  j, mtkwifi.__split(cfgs.VOW_Group_Max_Rate,";")[1])
        vifs[j].__atc_at     = mtkwifi.token_get(cfgs.VOW_Airtime_Ctrl_En, j, mtkwifi.__split(cfgs.VOW_Airtime_Ctrl_En,";")[1])
        vifs[j].__atc_min_at = mtkwifi.token_get(cfgs.VOW_Group_Min_Ratio, j, mtkwifi.__split(cfgs.VOW_Group_Min_Ratio,";")[1])
        vifs[j].__atc_max_at = mtkwifi.token_get(cfgs.VOW_Group_Max_Ratio, j, mtkwifi.__split(cfgs.VOW_Group_Max_Ratio,";")[1])

        -- TODO index by vifname
        vifs[vifs[j].vifname] = vifs[j]

        -- OFDMA and MU-MIMO
        vifs[j].__muofdma_dlenable = mtkwifi.token_get(cfgs.MuOfdmaDlEnable, j, mtkwifi.__split(cfgs.MuOfdmaDlEnable,";")[1])
        vifs[j].__muofdma_ulenable = mtkwifi.token_get(cfgs.MuOfdmaUlEnable, j, mtkwifi.__split(cfgs.MuOfdmaUlEnable,";")[1])
        vifs[j].__mumimo_dlenable = mtkwifi.token_get(cfgs.MuMimoDlEnable, j, mtkwifi.__split(cfgs.MuMimoDlEnable,";")[1])
        vifs[j].__mumimo_ulenable = mtkwifi.token_get(cfgs.MuMimoUlEnable, j, mtkwifi.__split(cfgs.MuMimoUlEnable,";")[1])
        vifs[j].__dtim_period = mtkwifi.token_get(cfgs.DtimPeriod, j, 1)
        vifs[j].__pmfmfpc = mtkwifi.token_get(cfgs.PMFMFPC, j, mtkwifi.__split(cfgs.PMFMFPC,";")[1])
        vifs[j].__pmfmfpr = mtkwifi.token_get(cfgs.PMFMFPR, j, mtkwifi.__split(cfgs.PMFMFPR,";")[1])
        vifs[j].__pmfsha256 = mtkwifi.token_get(cfgs.PMFSHA256, j, mtkwifi.__split(cfgs.PMFSHA256,";")[1])
        vifs[j].__disable = mtkwifi.token_get(cfgs.ApEnable, j, mtkwifi.__split(cfgs.ApEnable,";")[1])
    end

    return vifs
end

function mtkwifi.__setup_apcli(cfgs, devname, mainidx, subidx)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local apcli = {}
    local dev_idx = string.match(devname, "(%w+)")
    local apcli_prefix = l1dat and l1dat[dridx][devname].apcli_ifname or
                         dbdc_apcli_prefix[mainidx][subidx]

    local apcli_name = apcli_prefix.."0"

    if mtkwifi.exists("/sys/class/net/"..apcli_name) then
        apcli.vifname = apcli_name
        apcli.devname = apcli_name
        apcli.vifidx = "1"
        local rd_pipe_output = mtkwifi.read_pipe("iwconfig "..apcli_name.." | grep ESSID 2>/dev/null")
        local ssid = rd_pipe_output and string.match(rd_pipe_output, "ESSID:\"(.*)\"")
        if not ssid or ssid == "" then
            apcli.status = "Disconnected"
        else
            apcli.ssid = ssid
            apcli.status = "Connected"
        end
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..apcli_name.."/flags 2>/dev/null")) or 0
        apcli.state = flags%2 == 1 and "up" or "down"
        rd_pipe_output = mtkwifi.read_pipe("cat /sys/class/net/"..apcli_name.."/address 2>/dev/null")
        apcli.mac_addr = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "?"
        rd_pipe_output = mtkwifi.read_pipe("iwconfig "..apcli_name.." | grep 'Access Point' 2>/dev/null")
        apcli.bssid = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "Not-Associated"
        return apcli
    else
        return
    end
end

function mtkwifi.__setup_eths()
    local etherInfo = {}
    local all_eth_devs = mtkwifi.read_pipe("ls /sys/class/net/ | grep eth | grep -v grep")
    if not all_eth_devs or all_eth_devs == "" then
        return
    end
    for ethName in string.gmatch(all_eth_devs, "(eth%d)") do
        local ethInfo = {}
        ethInfo['ifname'] = ethName
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..ethName.."/flags 2>/dev/null")) or 0
        ethInfo['state'] = flags%2 == 1 and "up" or "down"
        ethInfo['mac_addr'] = mtkwifi.read_pipe("cat /sys/class/net/"..ethName.."/address 2>/dev/null") or "?"
        table.insert(etherInfo,ethInfo)
    end
    return etherInfo
end

function mtkwifi.__is_6890_project()
    local str = mtkwifi.read_pipe("cat /etc/vendor_info | grep \"PLATFORM=\"")
    str = string.gsub(str, "PLATFORM=", "")
    if str:find("6890") then
        return true
    end
    return false
end

function mtkwifi._is_support_6G()
    local support_6e = false
    local tmp = io.popen("cat /etc/map/1905d.cfg | grep radio_band")
    local result = tmp:read("*all")
    if result and string.find(result, "6G") then
        support_6e = true
    end
    tmp:close()
    return support_6e
end

function mtkwifi.channel_region_support()
	local support_channel_region = true
    local tmp = io.popen("cat /etc/wireless/mediatek/mt7990.1.dat | grep UnifiedCountryConfig")
    local result = tmp:read("*all")
    if result and string.find(result, "UnifiedCountryConfig=1") then
        support_channel_region = false
    end
    tmp:close()
    return support_channel_region
end

function mtkwifi.r6_support()
    local r6_support = false
    local fp = io.popen("1905ctrl dev show_r6_support")
    local result = fp:read("*all")
    fp:close()
    if result and (string.find(result, "MAP_R6_SUPPORT") or string.find(result, "MAP_PRE_R6_SUPPORT")) then
        r6_support = true
    end
    return r6_support
end

function mtkwifi.get_all_devs()
    local nixio = require("nixio")
    local ucicfg = mtkwifi.uci_load_wireless("wireless")
    local devs = {}
    local i = 1 -- dev idx
    local profiles = mtkwifi.search_dev_and_profile()
    local wpa_support = 0
    local wapi_support = 0

    for _, dev in pairs(ucicfg["wifi-device"]) do
        local devname = string.gsub(dev[".name"], "%_", ".")
        local cfgs = mtkwifi.load_cfg(devname)

        if not cfgs then
            debug_write("error loading devname"..devname)
            nixio.syslog("err", "error loading "..devname)
            return
        end
        devs[i] = {}
        devs[i].htmode=cfgs.Htmode
        devs[i].vifs = {}
        devs[i].apcli = {}
        devs[i].devname = devname
        --devs[i].profile = profile
        local tmp = ""
        tmp = string.split(devname, ".")
        devs[i].maindev = tmp[1]
        devs[i].mainidx = tonumber(tmp[2]) or 1
        devs[i].subdev = devname
        devs[i].subidx = string.match(tmp[3] or "", "(%d+)")=="2" and 2 or 1
        devs[i].devband = tonumber(tmp[3])
        if devs[i].devband then
            devs[i].multiprofile = true
            devs[i].dbdc = true
            --devs[i].dbdcBandName = (profile:match("2[gG]") and "2.4G") or (profile:match("5[gG]") and "5G")
            if not devs[i].dbdcBandName then
                -- Make 1st band as 2.4G and 2nd band as 5G.
                devs[i].dbdcBandName = (devs[i].devband == 1) and "2.4G" or "5G"
            end
        end

        devs[i].ApCliEnable = cfgs.ApCliEnable
        devs[i].WirelessMode = string.split(cfgs.WirelessMode,";")[1]
        devs[i].WirelessModeList = {}
        for key, value in pairs(DevicePropertyMap) do
            local found = string.find(string.upper(devname), string.upper(value.device))
            if found then
                for k=1,#value.band do
                    devs[i].WirelessModeList[tonumber(value.band[k])] = WirelessModeList[tonumber(value.band[k])]
                end

                if mtkwifi.__is_6890_project() then
                    if devs[i].dbdc then
                        nixio.syslog("debug", "6890 MiFi, change maxVif to 4")
                        devs[i].maxVif = 4
                    else
                        nixio.syslog("debug", "6890 CPE, change maxVif to 8")
                        devs[i].maxVif = 8
                    end
                elseif devs[i].dbdc == true then
                    devs[i].maxVif = value.maxDBDCVif or value.maxVif/2
                else
                    devs[i].maxVif = value.maxVif or 16
                end

                devs[i].maxTxStream = value.maxTxStream
                devs[i].maxRxStream = value.maxRxStream
                devs[i].invalidChBwList = value.invalidChBwList
                devs[i].isPowerBoostSupported = value.isPowerBoostSupported
                devs[i].wdsBand = value.wdsBand
                devs[i].mimoBand = value.mimoBand
                devs[i].isMultiAPSupported = value.isMultiAPSupported
                devs[i].isWPA3_192bitSupported = value.isWPA3_192bitSupported
            end
        end
        devs[i].WscConfMode = cfgs.WscConfMode
        devs[i].AuthModeList = AuthModeList
        devs[i].AuthModeList_6G = AuthModeList_6G
        devs[i].WpsEnableAuthModeList = WpsEnableAuthModeList
        devs[i].WpsEnableAuthModeList_6G = WpsEnableAuthModeList_6G

        if wpa_support == 1 then
            table.insert(devs[i].AuthModeList,"WPAPSK")
            table.insert(devs[i].AuthModeList,"WPA")
        end

        if wapi_support == 1 then
            table.insert(devs[i].AuthModeList,"WAIPSK")
            table.insert(devs[i].AuthModeList,"WAICERT")
        end
        devs[i].ApCliAuthModeList = ApCliAuthModeList
        devs[i].ApCliAuthModeList_6G = ApCliAuthModeList_6G
        devs[i].EncryptionTypeList = EncryptionTypeList
        devs[i].EncryptionTypeList_6G = EncryptionTypeList_6G
        devs[i].Channel = tonumber(cfgs.Channel)
        devs[i].DBDC_MODE = tonumber(cfgs.DBDC_MODE)
        devs[i].band = devs[i].devband or mtkwifi.band(string.split(cfgs.WirelessMode,";")[1])

        if cfgs.MUTxRxEnable then
            if tonumber(cfgs.ETxBfEnCond)==1
                and tonumber(cfgs.MUTxRxEnable)==0
                and tonumber(cfgs.ITxBfEn)==0
                then devs[i].__mimo = 0
            elseif tonumber(cfgs.ETxBfEnCond)==0
                and tonumber(cfgs.MUTxRxEnable)==0
                and tonumber(cfgs.ITxBfEn)==1
                then devs[i].__mimo = 1
            elseif tonumber(cfgs.ETxBfEnCond)==1
                and tonumber(cfgs.MUTxRxEnable)==0
                and tonumber(cfgs.ITxBfEn)==1
                then devs[i].__mimo = 2
            elseif tonumber(cfgs.ETxBfEnCond)==1
                and tonumber(cfgs.MUTxRxEnable)>0
                and tonumber(cfgs.ITxBfEn)==0
                then devs[i].__mimo = 3
            elseif tonumber(cfgs.ETxBfEnCond)==1
                and tonumber(cfgs.MUTxRxEnable)>0
                and tonumber(cfgs.ITxBfEn)==1
                then devs[i].__mimo = 4
            else devs[i].__mimo = 5
            end
        end

        if cfgs.HT_BW == "0" or not cfgs.HT_BW then
            devs[i].__bw = "20"
        elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "0" or not cfgs.VHT_BW then
            devs[i].__bw = "40"
        elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "1" then
            devs[i].__bw = "80"
        elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "2" then
            if cfgs.EHT_ApBw == '3' then
            devs[i].__bw = "160"
            elseif cfgs.EHT_ApBw == '4' then
                devs[i].__bw = "320"
            end
        elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "3" then
            devs[i].__bw = "161"
        end

        devs[i].vifs = mtkwifi.__setup_vifs(cfgs, devname, devs[i].mainidx, devs[i].subidx)
        devs[i].apcli = mtkwifi.__setup_apcli(cfgs, devname, devs[i].mainidx, devs[i].subidx)



        if mtkwifi.exists("cat /etc/wireless/"..devs[i].maindev.."/version") then
            local version = mtkwifi.read_pipe("cat /etc/wireless/"..devs[i].maindev.."/version 2>/dev/null")
            devs[i].version = (type(version) == "string" and version ~= "") and version or "Unknown: Empty version file!"
        else
            local vif_name = nil
            if devs[i].apcli and devs[i].apcli["state"] == "up" then
                vif_name = devs[i].apcli["vifname"]
            elseif devs[i].vifs then
                for _,vif in ipairs(devs[i].vifs) do
                    if vif["state"] == "up" then
                        vif_name = vif["vifname"]
                        break
                    end
                end
            end
            if not vif_name then
                if tonumber(cfgs.BssidNum) >= 1 then
                    devs[i].version = "Enable an interface to get the driver version."
                elseif devs[i].apcli and devs[i].apcli["state"] ~= "up" then
                    devs[i].version = "Enable ApCli interface i.e. "..devs[i].apcli["vifname"].." to get the driver version."
                else
                    devs[i].version = "Add an interface to get the driver version."
                end
            else

                --local version = mtkwifi.read_pipe("mwctl "..vif_name.." show driverinfo")
                --local str = os.execute("mwctl ra0 show driverinfo")

                --version = version and version:match("Driver version: (.-)\n") or ""
                -- devs[i].version = version ~= "" and version or "Unknown: Incorrect response from version command!"
                devs[i].version = get_drv_version(vif_name) or "Unknown: Incorrect response from version command!"
            end
        end

        -- Setup reverse indices by devname
        devs[devname] = devs[i]

        if devs[i].apcli then
            devs[i][devs[i].apcli.devname] = devs[i].apcli
        end

        i = i + 1
    end
    devs['etherInfo'] = mtkwifi.__setup_eths()
    return devs
end

function mtkwifi.exists(path)
    local fp = io.open(path, "rb")
    if fp then fp:close() end
    return fp ~= nil
end

function mtkwifi.parse_mac(str)
    local macs = {}
    local pat = "^[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]$"

    local function ismac(str)
        if str:match(pat) then return str end
    end

    if not str then return macs end
    local t = str:split("\n")
    for _,v in pairs(t) do
            local mac = ismac(mtkwifi.__trim(v))
            if mac then
                table.insert(macs, mac)
            end
    end

    return macs
    -- body
end

function divide_string(str)
    local arr = {}
    for s in string.gmatch(str, "%S+") do
        table.insert(arr,s)
    end
    return arr
end

function mtkwifi.scan_ap(vifname)
    local next_line_index = 0
    local i, line, cur_index
    local total_index = 0
    local ap_list = {}
    local xx = {}
    local tmp
    local j = 0
    local finished = false

    os.execute("mwctl "..vifname.." scan clear")
    os.execute("sleep 1")
    os.execute("mwctl "..vifname.." scan type=full")
    os.execute("sleep 25")
    while (true) do
        local fp = io.popen("mwctl "..vifname.." scan dump="..j)
        local scan_result = fp:read("*all")
        fp:close()

        for i, line in ipairs(mtkwifi.__lines(scan_result)) do
            local is_mac_addr_present = string.match(line, "%s+%x%x:%x%x:%x%x:%x%x:%x%x:%x%x%s+")
            -- If the line does not contain any MAC address and length is greater than 40 bytes,
            -- then, the line is the header of the get_site_survey page.

            -- "the scan result len is invalid !!!" or "BssInfo Idx(98) is out of range(0~97)"
            if string.find(line, "BssInfo Idx") or string.find(line, "the scan result len is invalid") then
                finished = true
                break
            end

            if #line>40 and not is_mac_addr_present then
                xx.No = {string.find(line, "No "),3}
                xx.Ch = {string.find(line, "Ch "),3}
                xx.SSID = {string.find(line, "SSID "),32}
                local fidx = string.find(line, "SSID_Len")
                if fidx then
                    xx.SSID_len = {fidx,2}
                end
                xx.BSSID = {string.find(line, "BSSID "),17}
                xx.Security = {string.find(line, "Security "),22}
                xx.Signal = {string.find(line, "Sig%a%al"),4}
                xx.Mode = {string.find(line, "W-Mode"),5}
                xx.ExtCh = {string.find(line, "ExtCH"),6}
                xx.WPS = {string.find(line, "WPS"),3}
                xx.NT = {string.find(line, "NT"),2}
                fidx = string.find(line, "OWETranIe")
                if fidx then
                    xx.OWETranIe = {fidx,9}
                end
            end

            if #line>40 and is_mac_addr_present then
                tmp = {}
                local arr = divide_string(line)
                j = tonumber(mtkwifi.__trim(arr[1]))+1
                tmp.channel = mtkwifi.__trim(arr[2])
                if string.match(arr[3], "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") then
                    tmp.ssid = "<Hidden SSID>"
                    tmp.bssid = string.upper(mtkwifi.__trim(arr[3]))
                    if  string.find(line, "GCMP256") then 
                        local s,e=string.find(arr[4], "GCMP256")
                        tmp.security = string.sub(mtkwifi.__trim(arr[4]),1,e)
                        tmp.authmode = mtkwifi.__trim(string.split(tmp.security, "/")[1])
                        tmp.encrypttype = mtkwifi.__trim(string.split(tmp.security, "/")[2] or "NONE")
                        if #arr[4]>e then
                            tmp.rssi = string.sub(mtkwifi.__trim(arr[4]),e+1)
                            tmp.extch = mtkwifi.__trim(arr[6])
                            tmp.mode = mtkwifi.__trim(arr[5])
                            tmp.wps = mtkwifi.__trim(arr[9])
                            tmp.nt = mtkwifi.__trim(arr[7])
                        else
                            tmp.rssi = mtkwifi.__trim(arr[5])
                            tmp.extch = mtkwifi.__trim(arr[7])
                            tmp.mode = mtkwifi.__trim(arr[6])
                            tmp.wps = mtkwifi.__trim(arr[10])
                            tmp.nt = mtkwifi.__trim(arr[8])
                        end
                    else
                        tmp.security = mtkwifi.__trim(arr[4])
                        tmp.authmode = mtkwifi.__trim(string.split(tmp.security, "/")[1])
                        tmp.encrypttype = mtkwifi.__trim(string.split(tmp.security, "/")[2] or "NONE")
                        tmp.rssi = mtkwifi.__trim(arr[5])
                        tmp.extch = mtkwifi.__trim(arr[7])
                        tmp.mode = mtkwifi.__trim(arr[6])
                        tmp.wps = mtkwifi.__trim(arr[10])
                        tmp.nt = mtkwifi.__trim(arr[8])
                    end
                else
                    tmp.ssid = mtkwifi.__trim(arr[3])
                    tmp.bssid = string.upper(mtkwifi.__trim(arr[4]))
                    if string.find(line, "GCMP256") then
                        local s,e=string.find(arr[5], "GCMP256")
                        tmp.security = string.sub(mtkwifi.__trim(arr[5]),1,e)
                        tmp.authmode = mtkwifi.__trim(string.split(tmp.security, "/")[1])
                        tmp.encrypttype = mtkwifi.__trim(string.split(tmp.security, "/")[2] or "NONE")
                        if #arr[5]>e then
                            tmp.rssi = string.sub(mtkwifi.__trim(arr[5]),e+1)
                            tmp.extch = mtkwifi.__trim(arr[7])
                            tmp.mode = mtkwifi.__trim(arr[6])
                            tmp.wps = mtkwifi.__trim(arr[10])
                            tmp.nt = mtkwifi.__trim(arr[8])
                        else
                            tmp.rssi = mtkwifi.__trim(arr[6])
                            tmp.extch = mtkwifi.__trim(arr[8])
                            tmp.mode = mtkwifi.__trim(arr[7])
                            tmp.wps = mtkwifi.__trim(arr[11])
                            tmp.nt = mtkwifi.__trim(arr[9])
                        end
                    else
                        tmp.security = mtkwifi.__trim(arr[5])
                        tmp.authmode = mtkwifi.__trim(string.split(tmp.security, "/")[1])
                        tmp.encrypttype = mtkwifi.__trim(string.split(tmp.security, "/")[2] or "NONE")
                        tmp.rssi = mtkwifi.__trim(arr[6])
                        tmp.extch = mtkwifi.__trim(arr[8])
                        tmp.mode = mtkwifi.__trim(arr[7])
                        tmp.wps = mtkwifi.__trim(arr[11])
                        tmp.nt = mtkwifi.__trim(arr[9])
                    end
                end
                table.insert(ap_list, tmp)
            end
        end

        if finished then
            break
        end
    end
    return ap_list
end

function mtkwifi.get_sta_list(vifname)
    local fp = io.popen("hostapd_cli -i "..vifname.." all_sta")
    local result = fp:read("*all")
    local sta_list = {}
    local list = {}
    fp:close()

    if result then
        for i, line in ipairs(mtkwifi.__lines(result)) do
            if line and #line ~= 0 then
                local i = string.find(line, "=")
                local k,v
                if i then
                    k = string.sub(line, 1, i-1)
                    v = string.sub(line, i+1)
                else
                    k = 'Mac'
                    v = line
                end
                list[k] = v
                if k == 'connected_time' then
                    table.insert(sta_list, list)
                    list = {}
                end
            end
        end
    end

    return sta_list
end

function mtkwifi.__any_wsc_enabled(wsc_conf_mode)
    if (wsc_conf_mode == "") then
        return 0;
    end
    if (wsc_conf_mode == "7") then
        return 1;
    end
    if (wsc_conf_mode == "4") then
        return 1;
    end
    if (wsc_conf_mode == "2") then
        return 1;
    end
    if (wsc_conf_mode == "1") then
        return 1;
    end
    return 0;
end

function mtkwifi.__restart_if_wps(devname, ifname, cfgs)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx
    local wsc_conf_mode = ""

    wsc_conf_mode=mtkwifi.token_get(cfgs["WscConfMode"], ssid_index, "")

    os.execute("route delete 239.255.255.250")
    debug_write("route delete 239.255.255.250")
    if(mtkwifi.__any_wsc_enabled(wsc_conf_mode)) then
        os.execute("route add -host 239.255.255.250 dev br0")
        debug_write("route add -host 239.255.255.250 dev br0")
    end

    return cfgs
end

function mtkwifi.restart_8021x(devname, devices)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local devs = devices or mtkwifi.get_all_devs()
    local dev = devs[devname]
    local main_ifname = l1dat and l1dat[dridx][devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
    local prefix = l1dat and l1dat[dridx][devname].ext_ifname or dbdc_prefix[mainidx][subidx]

    local ps_cmd = "ps | grep -v grep | grep rt2860apd | grep "..main_ifname.." | awk '{print $1}'"
    local pid_cmd = "cat /var/run/rt2860apd_"..devs[devname].vifs[1].vifname..".pid"
    local apd_pid = mtkwifi.read_pipe(pid_cmd) or mtkwifi.read_pipe(ps_cmd)
    if tonumber(apd_pid) then
        os.execute("kill "..apd_pid)
    end

    local cfgs = mtkwifi.load_profile(devs[devname].profile)
    local auth_mode = cfgs['AuthMode']
    local ieee8021x = cfgs['IEEE8021X']
    local pat_auth_mode = {"WPA$", "WPA;", "WPA2$", "WPA2;", "WPA1WPA2$", "WPA1WPA2;"}
    local pat_ieee8021x = {"1$", "1;"}
    local apd_en = false

    for _, pat in ipairs(pat_auth_mode) do
        if string.find(auth_mode, pat) then
            apd_en = true
        end
    end

    for _, pat in ipairs(pat_ieee8021x) do
        if string.find(ieee8021x, pat) then
            apd_en = true
        end
    end

    if not apd_en then
        return
    end
    if prefix == "ra" then
        mtkwifi.__fork_exec("rt2860apd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rae" then
        mtkwifi.__fork_exec("rtwifi3apd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rai" then
        mtkwifi.__fork_exec("rtinicapd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rax" or prefix == "ray" or prefix == "raz" then
        mtkwifi.__fork_exec("rt2860apd_x -i "..main_ifname.." -p "..prefix)
    end
end

function mtkwifi.get_referer_url()
    local to_url
    local script_name = luci.http.getenv('SCRIPT_NAME')
    local http_referer = luci.http.getenv('HTTP_REFERER')
    if script_name and http_referer then
        local fIdx = http_referer:find(script_name,1,true)
        if fIdx then
            to_url = http_referer:sub(fIdx)
        end
    end
    if not to_url or to_url == "" then
        to_url = luci.dispatcher.build_url("admin", "mtk", "wifi")
    end
    return to_url
end

function mtkwifi.save_read_easymesh_profile()
    local easymesh_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__read_easymesh_profile_path())
    if not mtkwifi.exists(easymesh_applied_path) then
        os.execute("cp -f "..mtkwifi.__read_easymesh_profile_path().." "..easymesh_applied_path)
    end

    local fd = io.open(mtkwifi.__read_easymesh_profile_path(), "w")
    if not fd then return end
    os.execute("lua /etc/uci2map.lua")
    fd:close()

    mtkwifi.save_easymesh_profile_to_nvram()
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.save_easymesh_profile_to_nvram()
    if not pcall(require, "mtknvram") then
        return
    end
    local nvram = require("mtknvram")
    local merged_easymesh_dev1_path = "/tmp/mtk/wifi/merged_easymesh_dev1.dat"
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dev1_profile_paths
    local dev1_profile_path_table = l1 and l1.l1_zone_to_path("dev00")
    if not next(dev1_profile_path_table) then
        return
    end
    dev1_profile_paths = table.concat(dev1_profile_path_table, " ")
    -- Uncomment below two statements when there is sufficient space in dev1 NVRAM zone to store EasyMesh Agent's BSS Cfgs Settings.
    -- mtkwifi.__prepare_easymesh_bss_nvram_cfgs()
    -- os.execute("cat "..dev1_profile_paths.." "..mtkwifi.__read_easymesh_profile_path().." "..mtkwifi.__easymesh_bss_cfgs_nvram_path().." > "..merged_easymesh_dev1_path.." 2>/dev/null")
    -- Comment or remove below line once above requirement is met.
    os.execute("cat "..dev1_profile_paths.." "..mtkwifi.__read_easymesh_profile_path().." > "..merged_easymesh_dev1_path.." 2>/dev/null")
    nvram.nvram_save_profile(merged_easymesh_dev1_path, "dev00")
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.save_write_easymesh_profile(easymesh_mapd_cfgs)
    if not easymesh_mapd_cfgs then
        return
    end
    local mapd_user = mtkwifi.read_pipe("uci show mapd.mapd_user") 
    if not mapd_user or mapd_user == "" then return end
    table.sort(easymesh_mapd_cfgs, function(a,b) return a<b end)
    for k,v in mtkwifi.__spairs(easymesh_mapd_cfgs, function(a,b) return string.upper(a) < string.upper(b) end) do
        os.execute("uci set mapd.mapd_user."..k.."="..v)
        os.execute("uci set mapd.mapd_cfg."..k.."="..v)
    end
    os.execute("uci commit")
    os.execute("lua /etc/uci2map.lua")
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.__read_easymesh_profile_path()
    return "/etc/map/mapd_cfg"
end

function mtkwifi.__write_easymesh_profile_path()
    return "/etc/map/mapd_user.cfg"
end

function mtkwifi.__easymesh_mapd_profile_path()
    return "/etc/mapd_strng.conf"
end

function mtkwifi.__easymesh_bss_cfgs_path()
    return "/etc/map/wts_bss_info_config"
end

function mtkwifi.__easymesh_bsta_mlo_cfgs_path()
    return "/etc/map/wts_bsta_mlo_config"
end

function mtkwifi.__easymesh_bss_cfgs_nvram_path()
    local p = "/tmp/mtk/wifi/wts_bss_info_config.nvram"
    os.execute("mkdir -p /tmp/mtk/wifi")
    return p
end

function mtkwifi.get_easymesh_al_mac(devRole)
    local r = {}
    local mapd_app_cfgs = mtkwifi.load_profile("/etc/map/1905d.cfg")
    if not mapd_app_cfgs then
        r['status'] = "Failed to load /etc/map/1905d.cfg file!"
    else
        r['status'] = 'SUCCESS'
        if tonumber(devRole) == 1 then
            r['al_mac'] = mapd_app_cfgs['map_controller_alid']
        else
            r['al_mac'] = mapd_app_cfgs['map_agent_alid']
        end
    end
    return r
end

function mtkwifi.get_easymesh_on_boarded_iface_info()
    local r = {}
    r['status'] = "ERROR"
    r['staBhInfStr'] = ""
    r['devname'] = ""
    local devs = mtkwifi.get_all_devs()
    for _, dev in ipairs(devs) do
        if dev.apcli and dev.apcli.status == "Connected" then
            r['status'] = "SUCCESS"
            r['staBhInfStr'] = r['staBhInfStr']..dev.apcli.vifname..';'
            r['devname'] = r['devname']..dev.devname..';'
        end
    end
    return r
end

function mtkwifi.load_easymesh_sta_mlo_cfgs()
    local shuci = require("shuci")
    local path = "/etc/config/mapd"
    if not mtkwifi.exists(path) then
        return
    end
    local mapd = shuci.decode(path)
    local cfgs = {}
    -- convert profile into lua table
    if mapd["bh_sta"] then
        for _, bh_sta in pairs(mapd["bh_sta"]) do
            staMac = bh_sta.almac
            cfgs[staMac] = {}
            cfgs[staMac]['name'] = bh_sta[".name"]
            cfgs[staMac]['mlo_links'] = bh_sta.mlo_links
            cfgs[staMac]['ruid_or_band'] = bh_sta.ruid_or_band
        end
    end
    
    return cfgs
end

function mtkwifi.load_easymesh_bss_cfgs()
    local shuci = require("shuci")
    local path = "/etc/config/mapd"
    local ssid,pwd
    if not mtkwifi.exists(path) then
        return
    end
    local mapd = shuci.decode(path)

    local cfgs = {}
    cfgs['wildCardAlMacCfgs'] = {}
    cfgs['distinctAlMacCfgs'] = {}
    local tmp = {}

    -- convert profile into lua table
    if mapd["iface"] then
        for _, iface in pairs(mapd["iface"]) do
            -- Trim only leading space characters
            local alMac, band = iface.mac, iface.radio
                if band then
                    alMac = alMac:upper()
                    local bssInfoIdx
                    if tmp[alMac] then
                        if tmp[alMac][band] then
                            bssInfoIdx = mtkwifi.get_table_length(tmp[alMac][band]) + 1
                            tmp[alMac][band][bssInfoIdx] = {}
                        else
                            bssInfoIdx = 1
                            tmp[alMac][band] = {}
                            tmp[alMac][band][bssInfoIdx] = {}
                        end
                    else
                        bssInfoIdx = 1
                        tmp[alMac] = {}
                        tmp[alMac][band] = {}
                        tmp[alMac][band][bssInfoIdx] = {}
                    end
                    tmp[alMac][band][bssInfoIdx]['id'] = iface[".name"]
                    ssid = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd."..iface[".name"]..".ssid"))
                    tmp[alMac][band][bssInfoIdx]['ssid'] = ssid:gsub("\\", "\\\\")
                    tmp[alMac][band][bssInfoIdx]['authMode'] = iface.authmode
                    tmp[alMac][band][bssInfoIdx]['encType'] = iface.EncryptType
                    pwd = mtkwifi.__trim(mtkwifi.read_pipe("uci get mapd."..iface[".name"]..".PSK")) or ""
                    tmp[alMac][band][bssInfoIdx]['passPhrase'] = pwd:gsub("\\", "\\\\")
                    tmp[alMac][band][bssInfoIdx]['isBhBssSupported'] = iface.bhbss
                    tmp[alMac][band][bssInfoIdx]['isFhBssSupported'] = iface.fhbss
                    tmp[alMac][band][bssInfoIdx]['isHidden'] = iface.hidden
                    tmp[alMac][band][bssInfoIdx]['fhVlanId'] = iface.vlan
                    tmp[alMac][band][bssInfoIdx]['primVlan'] = iface.pvid
                    tmp[alMac][band][bssInfoIdx]['defPCP'] = iface.pcp
                    tmp[alMac][band][bssInfoIdx]['mldGroupId'] = iface.mld_groupID

                    if alMac == "FF:FF:FF:FF:FF:FF" then
                        cfgs['wildCardAlMacCfgs']['FF:FF:FF:FF:FF:FF'] = tmp[alMac]
                    else
                        cfgs['distinctAlMacCfgs'][alMac] = tmp[alMac]
                    end

                else
                    nixio.syslog("warning", "load_easymesh_bss_cfgs: skip line without 'LineNumber,AL-MAC Band' "..line)
                end
        end
    end
    return cfgs
end

function mtkwifi.save_easymesh_bss_cfgs()
    local easymesh_bss_cfg_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__easymesh_bss_cfgs_path())
    if not mtkwifi.exists(easymesh_bss_cfg_applied_path) then
        os.execute("cp -f "..mtkwifi.__easymesh_bss_cfgs_path().." "..easymesh_bss_cfg_applied_path)
    end

    os.execute("lua /etc/uci2map.lua")

    os.execute("sync "..mtkwifi.__easymesh_bss_cfgs_path().."  >/dev/null 2>&1")

    -- Uncomment below line when there is sufficient space in dev1 NVRAM zone to store EasyMesh Agent's BSS Cfgs Settings.
    -- mtkwifi.save_easymesh_profile_to_nvram()
end

function mtkwifi.__prepare_easymesh_bss_nvram_cfgs()
    local fd = io.open(mtkwifi.__easymesh_bss_cfgs_nvram_path(), "w")
    if not fd then
        return
    end
    local cfgs = mtkwifi.load_easymesh_bss_cfgs()
    local lineIdx = 0
    -- First write distinct AL-MAC cfgs; then write wildcard AL-MAC(FF:FF:FF:FF:FF:FF) cfgs
    for alMac,alMacTbl in pairs(cfgs['distinctAlMacCfgs']) do
        for band,bssInfoTbl in pairs(alMacTbl) do
            for _,bssInfo in pairs(bssInfoTbl) do
                lineIdx = lineIdx + 1
                fd:write('EasyMeshBssCfgsLine'..lineIdx..'='..lineIdx..','..alMac..' '..
                    band..' '..
                    bssInfo['ssid']..' '..
                    bssInfo['authMode']..' '..
                    bssInfo['encType']..' '..
                    bssInfo['passPhrase']..' '..
                    bssInfo['isBhBssSupported']..' '..
                    bssInfo['isFhBssSupported']..' '..
                    bssInfo['isHidden']..' '..
                    bssInfo['fhVlanId']..' '..
                    bssInfo['primVlan']..' '..
                    bssInfo['defPCP']..
                    '\n')
            end
        end
    end
    for alMac,alMacTbl in pairs(cfgs['wildCardAlMacCfgs']) do
        for band,bssInfoTbl in pairs(alMacTbl) do
            for _,bssInfo in pairs(bssInfoTbl) do
                lineIdx = lineIdx + 1
                fd:write('EasyMeshBssCfgsLine'..lineIdx..'='..lineIdx..','..alMac..' '..
                    band..' '..
                    bssInfo['ssid']..' '..
                    bssInfo['authMode']..' '..
                    bssInfo['encType']..' '..
                    bssInfo['passPhrase']..' '..
                    bssInfo['isBhBssSupported']..' '..
                    bssInfo['isFhBssSupported']..' '..
                    bssInfo['isHidden']..' '..
                    bssInfo['fhVlanId']..' '..
                    bssInfo['primVlan']..' '..
                    bssInfo['defPCP']..
                    '\n')
            end
        end
    end
    fd:write('EasyMeshTotalBssCfgsLines='..lineIdx..'\n')
    fd:close()
    os.execute("sync >/dev/null 2>&1")
end

return mtkwifi
