#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path


local mtkdat = require("mtkdat")
local nixio = require("nixio")
local hostapd = require("hostapd")
local supplicant = require("supplicant")

local function esc(x)
   return (x:gsub('%%', '%%%%')
            :gsub('^%^', '%%^')
            :gsub('%$$', '%%$')
            :gsub('%(', '%%(')
            :gsub('%)', '%%)')
            :gsub('%.', '%%.')
            :gsub('%[', '%%[')
            :gsub('%]', '%%]')
            :gsub('%*', '%%*')
            :gsub('%+', '%%+')
            :gsub('%-', '%%-')
            :gsub('%?', '%%?'))
end

local function add_vif_into_net(vif, net)
    if net == nil then
        return
    end
    nixio.syslog("debug", "add "..vif.." into br-"..net)
    os.execute("ubus -t 20 wait_for network.interface."..net..
        " && ubus call network.interface."..net.." add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
    if mtkdat.exist("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
        os.execute("echo 1 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
    end
end


local function del_vif_from_net(vif)
    for _,f in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net/"..vif.." 2> /dev/null"), "\n")) do
        if string.match(f, "upper_(%a+)") then
            if mtkdat.exist("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
                os.execute("echo 0 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
            end
            local bridge = string.split(f, "_")[2]
            if bridge then
                nixio.syslog("debug", "remove "..vif.." into "..bridge)

                local net = string.split(bridge, "-")[2]
                if net then
                    os.execute("ubus call network.interface."..net.." remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
                end
                os.execute("brctl delif "..bridge.." "..vif.." 2> /dev/null")
                return
            end
        end
    end
end

function cfg80211_tool()
    if mtkdat.exist("/usr/sbin/mwctl") then
        return "/usr/sbin/mwctl"
    end
    if mtkdat.exist("/usr/sbin/iwpriv") then
        return "/usr/sbin/iwpriv"
    end
    return nil
end

function mtwifi_setup_apcli_vif_preinit(uci_dev, uci_vif)
    if uci_vif.mwds ~= nil then
        if uci_vif.mwds ~= "0" then
            os.execute(cfg80211_tool().." "..uci_vif[".name"].." set mwds enable=1")
        else
            os.execute(cfg80211_tool().." "..uci_vif[".name"].." set mwds enable=0")
        end
    end
end

function mtwifi_setup_apcli_vif_postinit(uci_dev, uci_vif)
end

function mtwifi_setup_ap_vif_postinit(uci_dev, uci_vif)
    if uci_vif.mwds ~= nil then
        if uci_vif.mwds ~= "0" then
            os.execute(cfg80211_tool().." "..uci_vif[".name"].." set mwds enable=1")
        else
            os.execute(cfg80211_tool().." "..uci_vif[".name"].." set mwds enable=0")
        end
    end
end

function mtwifi_up(devname)
    local wifi_services_exist = false
    local l1profile = "/etc/wireless/l1profile.dat"
    local uci = mtkdat.uci_load_wireless()

    if devname then
        local profiles = mtkdat.search_dev_and_profile()
        local path = profiles[devname]
        if not mtkdat.exist("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last") then
            os.execute("cp -f "..path.." "..mtkdat.__profile_previous_settings_path(path))
        end
    end

    local path = "/etc/config/wireless"
    os.execute("cp -f "..path.." "..mtkdat.__uci_applied_settings_path())

    if mtkdat.cfg_is_diff() then
        mtkdat.uci2dat()
    end

    if  mtkdat.exist("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mtwifi called!")

    local devs, l1parser = mtkdat.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mtwifi: dev "..devname.." not found!")
            return
        end

        local devname2 = string.gsub(devname, "%.", "_")
        local uci_dev = mtkdat.get_uci_dev_by_dev_name(uci, devname2)
        local uci_main_vif = mtkdat.get_uci_vif_by_vif_name(uci, dev.main_ifname)
        local profile = mtkdat.search_dev_and_profile()[devname]
        local cfgs = mtkdat.load_profile(profile)
        -- we have to bring up main_ifname first, main_ifname will create all other vifs.
        if mtkdat.exist("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("debug", "mtwifi_up: ifconfig "..dev.main_ifname.." up")
            if not mtkdat.exist("/etc/init.d/wpad") or ( dev.TestModeEn ~= nil and dev.TestModeEn ~= "0" ) then
                if (uci_dev.disabled == nil or uci_dev.disabled == '0') and
                   (uci_main_vif.disabled == nil or uci_main_vif.disabled == '0') then
                    nixio.syslog("debug", "mtwifi_up: ifconfig "..dev.main_ifname.." up")
                    os.execute("ifconfig "..dev.main_ifname.." up")
                end
                add_vif_into_net(dev.main_ifname, uci_main_vif.network)
            else
                nixio.syslog("debug", "mtwifi_up: ifconfig "..dev.main_ifname.." up")
                os.execute("ifconfig "..dev.main_ifname.." up")
                if uci_main_vif.mode == "ap" then
                    hostapd_setup_vif(uci_dev, uci_main_vif)
                    if (uci_dev.disabled == nil or uci_dev.disabled == '0') and
                       (uci_main_vif.disabled == nil or uci_main_vif.disabled == '0') then
                        hostapd_enable_vif(dev.main_ifname, uci_main_vif)
                    end
                    set_mld_bcn_down()
                    add_vif_into_net(dev.main_ifname, uci_main_vif.network)
                end
            end
            mtwifi_setup_ap_vif_postinit(uci_dev, uci_main_vif)
        else
            nixio.syslog("err", "mtwifi_up: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        for _,vif in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif ~= dev.main_ifname then
                local uci_vif = mtkdat.get_uci_vif_by_vif_name(uci, vif)

                if uci_vif ~= nil then
                    if string.match(vif, esc(dev.apcli_ifname).."[0-9]+") then
                        mtwifi_setup_apcli_vif_preinit(uci_dev, uci_vif)
                        if not mtkdat.exist("/etc/init.d/wpad") or ( dev.TestModeEn ~= nil and dev.TestModeEn ~= "0" ) then
                            nixio.syslog("debug", "mtwifi_up: ifconfig "..vif.." up")
                            os.execute("ifconfig "..vif.." up")
                            add_vif_into_net(vif, uci_vif.network)
                        else
                            if uci_vif.mode == "sta" then
                                add_vif_into_net(vif, uci_vif.network)
                                supp_setup_vif(uci_dev, uci_vif)

                                if (uci_dev.disabled == nil or uci_dev.disabled == '0') then
                                    local uci_mld = mtkdat.get_uci_mld_by_vif_name(uci, vif)

                                    if uci_mld ~= nil and ( uci_mld.disabled == nil or uci_mld.disabled == '0' ) then
                                        local uci_main_vif = mtkdat.get_uci_sta_mld_main_iface(uci, uci_mld)
                                        if uci_main_vif[".name"] ~= uci_vif[".name"] then
                                            supp_enable_vif(uci_vif)
                                            if (uci_vif.disabled ~= nil and uci_vif.disabled ~= '0') then
                                                os.execute("ifconfig "..vif.." down")
                                            end
                                        end
                                        if mtkdat.check_if_sta_mld_all_members_up(uci, uci_mld) ~= 0 then
                                            supp_enable_vif(uci_main_vif)
                                            if (uci_main_vif.disabled ~= nil and uci_main_vif.disabled ~= '0') then
                                                os.execute("ifconfig "..uci_main_vif[".name"].." down")
                                            end
                                        end
                                    else
                                        supp_enable_vif(uci_vif)
                                        if (uci_vif.disabled ~= nil and uci_vif.disabled ~= '0') then
                                            os.execute("ifconfig "..vif.." down")
                                        end
                                    end
                                end
                            end
                        end
                        mtwifi_setup_apcli_vif_postinit(uci_dev, uci_vif)
                    elseif (string.match(vif, esc(dev.wds_ifname).."[0-9]+") and
                            cfgs.WdsEnable ~= "0" and cfgs.WdsEnable ~= "") or
                           string.match(vif, esc(dev.mesh_ifname).."[0-9]+") then
                        nixio.syslog("debug", "mtwifi_up: ifconfig "..vif.." up")
                        os.execute("ifconfig "..vif.." up")
                        add_vif_into_net(vif, uci_vif.network)
                    end
                end
            -- else nixio.syslog("debug", "mtwifi_up: skip "..vif..", prefix not match "..pre)
            end
        end

        local uci_vifs = mtkdat.get_uci_vifs_by_dev_name(uci, devname2)
        if uci_vifs ~= nil then
            local uci_vif
            for _, uci_vif in pairs(uci_vifs)
            do
                if uci_vif[".name"] ~= dev.main_ifname and
                   string.match(uci_vif[".name"], esc(dev.ext_ifname).."[0-9]+") then
                    if uci_vif.mode == "ap" then
                        if not mtkdat.exist("/etc/init.d/wpad") or ( dev.TestModeEn ~= nil and dev.TestModeEn ~= "0" ) then
                            if uci_vif.disabled == nil or uci_vif.disabled ~= '1' then
                                os.execute("ifconfig "..uci_vif[".name"].." up")
                            end
                            add_vif_into_net(uci_vif[".name"], uci_vif.network)
                        else
                            local phyname = mtkdat.get_phy_by_main_ifname(dev.main_ifname)

                            os.execute("iw phy "..phyname.." interface add "..uci_vif[".name"].." type __ap 2> /dev/null")
                            hostapd_setup_vif(uci_dev, uci_vif)
                            if (uci_dev.disabled == nil or uci_dev.disabled == '0') and
                               (uci_vif.disabled == nil or uci_vif.disabled == '0') then
                                hostapd_enable_vif(dev.main_ifname, uci_vif)
                            end
                            add_vif_into_net(uci_vif[".name"], uci_vif.network)
                            set_mld_bcn_down()
                            mtwifi_setup_ap_vif_postinit(uci_dev, uci_vif)
                        end
                    end
                end
            end
        end

        if mtkdat.exist("/sys/class/net/"..dev.main_ifname) then
            if mtkdat.exist("/etc/init.d/wpad") and ( dev.TestModeEn == nil or dev.TestModeEn == "0" ) then
                if (uci_dev.disabled ~= nil and uci_dev.disabled ~= '0') or
                   (uci_main_vif.disabled ~= nil and uci_main_vif.disabled ~= '0') then
                    nixio.syslog("debug", "mtwifi_up: ifconfig "..dev.main_ifname.." down")
                    os.execute("ifconfig "..dev.main_ifname.." down")
                end
            end
        end
    else nixio.syslog("debug", "mtwifi_up: skip "..devname..", config(l1profile) not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mtwifi*.need_reload")
end

local function get_vifs_by_phy(phyname)
    local devs={}

    for filename in io.popen("ls /sys/class/net"):lines() do
        local parent=io.open("/sys/class/net/"..filename.."/phy80211/name")
        if parent ~= nil then
            local name=parent:read "*a"

            if string.match(name, phyname) then
                table.insert(devs, filename)
            end
            io.close(parent)
        end
    end
    return devs
end

function mtwifi_down(devname)
    os.execute("echo wifi down > /dev/console")
    if mtkdat.cfg_is_diff() then
        mtkdat.uci2dat()
    end

    local path = "/etc/config/wireless"
    os.execute("cp -f "..path.." "..mtkdat.__uci_applied_settings_path())

    local wifi_services_exist = false
    if  mtkdat.exist("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mtwifi_down called!")

    -- M.A.N service
    if mtkdat.exist("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
    end


    local devs, l1parser = mtkdat.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mtwifi_down: dev "..devname.." not found!")
            return
        end
        if not mtkdat.exist("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("err", "mtwifi_down: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        os.execute("mwctl "..dev.main_ifname.." set hw_nat_register=0")
        for _,vif in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net"), "\n"))
        do
            if string.match(vif, esc(dev.apcli_ifname).."[0-9]+") then
                os.execute("ifconfig "..vif.." down 2> /dev/null")
                supp_disable_vif(vif)
                del_vif_from_net(vif)
            end
        end
        for _,vif in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net"), "\n"))
        do
            if string.match(vif, esc(dev.ext_ifname).."[1-9]+") then
                hostapd_disable_vif(vif)
                os.execute("ifconfig "..vif.." down 2> /dev/null")
                del_vif_from_net(vif)
                os.execute("iw dev "..vif.." del 2> /dev/null")
            end
        end
        for _,vif in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net"), "\n"))
        do
            if string.match(vif, esc(dev.wds_ifname).."[0-9]+") or
                   string.match(vif, esc(dev.mesh_ifname).."[0-9]+") then
                nixio.syslog("debug", "mtwifi_down: ifconfig "..vif.." down")
                os.execute("ifconfig "..vif.." down 2> /dev/null")
                del_vif_from_net(vif)
            end
        end
        for _,vif in ipairs(string.split(mtkdat.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif == dev.main_ifname then
                hostapd_disable_vif(vif)
                os.execute("ifconfig "..vif.." down 2> /dev/null")
                del_vif_from_net(vif)
            end
        end
    else nixio.syslog("debug", "mtwifi_down: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mtwifi*.need_reload")
end

function mtwifi_reload(devname)
    local normal_reload = true
    local qsetting = false
    local cfgs, diff
    local path, profiles
    local devs, l1parser = mtkdat.__get_l1dat()
    nixio.syslog("debug", "mtwifi_reload called!")

    if mtkdat.exist("/lib/wifi/quick_setting.lua") then
        qsetting = true
        profiles = mtkdat.search_dev_and_profile()
    end

    for devname, dev in mtkdat.spairs(devs.devname_ridx) do
        if qsetting then
            -- Create devname.last for quick setting
            path = profiles[devname]
            os.execute("cp -f "..path..
                " "..mtkdat.__profile_previous_settings_path(path))
        end
    end

    local path = "/etc/config/wireless"
    os.execute("cp -f "..path.." "..mtkdat.__uci_applied_settings_path())

    if mtkdat.cfg_is_diff() then
        mtkdat.uci2dat()
    end

    -- For one card , all interface should be down, then up
    if not devname then
        local cfgs_table = {}
        local diff_table = {}
        if qsetting then
            for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
                _path = profiles[_devname]
                _dev.normal_reload, _cfgs, _diff = check_quick_settings(_devname, _path)
                if _dev.normal_reload == false then
                    normal_reload = false
                end
                cfgs_table[_devname] = _cfgs
                diff_table[_devname] = _diff
            end
        end

        if normal_reload then
            for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
                mtwifi_down(_devname)
            end
            for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
                mtwifi_up(_devname)
            end
        else
            for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
                if cfgs_table[_devname] and diff_table[_devname] then
                    do_quick_settings(_devname, cfgs_table[_devname], diff_table[_devname])
                end
            end
        end
    else
        if qsetting then
            path = profiles[devname]
            normal_reload, cfgs, diff = check_quick_settings(devname, path)
        end

        if normal_reload then
            local dev = devs.devname_ridx[devname]
            --assert(exist(dev.init_script))
            local compatname = dev.init_compatible
            -- Different cards do not affect each other
            if not string.find(dev.profile_path, "dbdc") then
                if dev.init_compatible == compatname then
                    mtwifi_down(devname)
                    mtwifi_up(devname)
                end
            --If the reloaded device belongs to dbdc, then another device on dbdc also need to be reloaded
            else
                for devname, dev in pairs(devs.devname_ridx) do
                    if dev.init_compatible == compatname then mtwifi_down(devname) end
                end
                for devname, dev in mtkdat.__spairs(devs.devname_ridx) do
                    if dev.init_compatible == compatname then mtwifi_up(devname) end
                end
            end
        else
            do_quick_settings(devname, cfgs, diff)
        end
    end
end

function mtwifi_restart(devname)
    local uci  = require "luci.model.uci".cursor()
    local devs, l1parser = mtkdat.__get_l1dat()

    if mtkdat.cfg_is_diff() then
        mtkdat.uci2dat()
    end

    local profiles = mtkdat.search_dev_and_profile()
    for devname, dev in mtkdat.spairs(devs.devname_ridx) do
        path = profiles[devname]
        os.execute("cp -f "..path..
            " "..mtkdat.__profile_previous_settings_path(path))
    end

    local path = "/etc/config/wireless"
    os.execute("cp -f "..path.." "..mtkdat.__uci_applied_settings_path())

    -- for AX8400 add 5G interface
    local isRoot = false
    if devname then
        local dev, path, diff
        local is7915 = false
        dev = devs.devname_ridx[devname]
        path = dev.profile_path
        if path then
            is7915 = string.find(path, "mt7915")
            diff =  mtkdat.diff_profile(path)
            if is7915 and diff.BssidNum then
                isRoot = true
            end
        end
    end

    nixio.syslog("debug", "mtwifi_restart called!")

    -- if wifi driver is built-in, it's necessary action to reboot the device
    if mtkdat.exist("/sys/module/mt_wifi") == false or isRoot then
        os.execute("echo reboot_required > /tmp/mtk/wifi/reboot_required")
        return
    end

    if devname then
        mtwifi_down(devname)
    else
--        for _devname, _dev in mtkdat.spairs(devs.devname_ridx, function(a,b) return string.upper(a) > string.upper(b) end) do
        for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
            mtwifi_down(_devname)
        end
    end
    os.execute("rmmod mt_whnat")
    os.execute("/etc/init.d/fwdd stop")
    os.execute("rmmod mtfwd")
    os.execute("rmmod mtk_warp_proxy")
    -- warp.ko actually no need to unload , due to sqc wifi restart may encounter the WO load fail
    -- keep the workaround until the warp / WO module init flow PASS in SQC env.
    -- os.execute("rmmod mtk_warp")
    -- mt7915_mt_wifi is for dual ko only
    os.execute("rmmod mt7915_mt_wifi")
    os.execute("rmmod mt_wifi")

    os.execute("modprobe mt_wifi")
    os.execute("modprobe mt7915_mt_wifi")
    -- os.execute("modprobe mtk_warp")
    os.execute("modprobe mtk_warp_proxy")
    os.execute("modprobe mtfwd")
    os.execute("/etc/init.d/fwdd start")
    os.execute("modprobe mt_whnat")
    if devname then
        mtwifi_up(devname)
    else
        for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
            mtwifi_up(_devname)
        end
    end
end

function mtwifi_reset(devname)
    nixio.syslog("debug", "mtwifi_reset called!")
    if mtkdat.exist("/rom/etc/wireless/mediatek/") then
        os.execute("rm -rf /etc/wireless/mediatek/")
        os.execute("cp -rf /rom/etc/wireless/mediatek/ /etc/wireless/")
        if mtkdat.exist("/etc/config/wireless") then
            os.execute("rm -rf /etc/config/wireless")
            mtwifi_detect(devname)
        end
        mtwifi_reload(devname)
    else
        nixio.syslog("debug", "mtwifi_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mtwifi_status(devname)
    return wifi_common_status()
end

function mtwifi_hello(devname)
   os.execute("echo mtwifi_hello: "..devname)
end

function mtwifi_detect(devname)
    nixio.syslog("debug", "mtwifi_detect")
    mtkdat.dat2uci()
end

function mtwifi_save(devname)
    mtkdat.uci2dat()
end

