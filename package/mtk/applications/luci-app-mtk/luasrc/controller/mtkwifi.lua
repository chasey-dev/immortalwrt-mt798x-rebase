

-- This module is a demo to configure MTK' proprietary WiFi driver.
-- Basic idea is to bypass uci and edit wireless profile (mt76xx.dat) directly.
-- LuCI's WiFi configuration is more logical and elegent, but it's quite tricky to
-- translate uci into MTK's WiFi profile (like we did in "uci2dat").
-- And you will get your hands dirty.
--
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path
module("luci.controller.mtkwifi", package.seeall)
local onboardingType = 0;
local ioctl_help = require "ioctl_helper"
local map_help
if pcall(require, "map_helper") then
    map_help = require "map_helper"
end
local http = require("luci.http")
local mtkwifi = require("mtkwifi")
local ucicfg = mtkwifi.uci_load_wireless("wireless")

local logDisable = 1
function debug_write(...)
    -- luci.http.write(...)
    if logDisable == 1 then
        return
    end
    local syslog_msg = "";
    local ff = io.open("/tmp/dbgmsg", "a")
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

function index()
    -- if not nixio.fs.access("/etc/wireless") then
    --     return
    -- end

    entry({"admin", "mtk"}, firstchild(), _("MTK"), 80)
    entry({"admin", "mtk", "test"}, call("test"))
    entry({"admin", "mtk", "wifi"}, template("admin_mtk/mtk_wifi_overview"), _("WiFi configuration"), 1)
    entry({"admin", "mtk", "wifi", "chip_cfg_view"}, template("admin_mtk/mtk_wifi_chip_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "chip_cfg"}, call("chip_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "dev_cfg_view"}, template("admin_mtk/mtk_wifi_dev_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "dev_cfg"}, call("dev_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "dev_cfg_raw"}, call("dev_cfg_raw")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_cfg_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_cfg"}, call("vif_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_add_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_add"}, call("vif_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_del"}, call("vif_del")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_disable"}, call("vif_disable")).leaf = true
    entry({"admin", "mtk", "wifi", "vif_enable"}, call("vif_enable")).leaf = true
    entry({"admin", "mtk", "wifi", "get_station_list"}, call("get_station_list"))
    entry({"admin", "mtk", "wifi", "get_country_region_list"}, call("get_country_region_list")).leaf = true
    entry({"admin", "mtk", "wifi", "get_channel_list"}, call("get_channel_list"))
    entry({"admin", "mtk", "wifi", "get_channel_list_not_support_region"}, call("get_channel_list_not_support_region"))
    entry({"admin", "mtk", "wifi", "get_wirelessmode"}, call("get_wirelessmode"))
    entry({"admin", "mtk", "wifi", "get_HT_ext_channel_list"}, call("get_HT_ext_channel_list"))
    entry({"admin", "mtk", "wifi", "get_5G_2nd_80Mhz_channel_list"}, call("get_5G_2nd_80Mhz_channel_list"))
    entry({"admin", "mtk", "wifi", "reset"}, call("reset_wifi")).leaf = true
    entry({"admin", "mtk", "wifi", "reload"}, call("reload_wifi")).leaf = true
    entry({"admin", "mtk", "wifi", "get_raw_profile"}, call("get_raw_profile"))
    entry({"admin", "mtk", "wifi", "apcli_cfg_view"}, template("admin_mtk/mtk_wifi_apcli")).leaf = true
    entry({"admin", "mtk", "wifi", "apcli_cfg"}, call("apcli_cfg")).leaf = true
    entry({"admin", "mtk", "wifi", "apcli_disconnect"}, call("apcli_disconnect")).leaf = true
    entry({"admin", "mtk", "wifi", "apcli_connect"}, call("apcli_connect")).leaf = true
    entry({"admin", "mtk", "netmode", "net_cfg"}, call("net_cfg"))
    entry({"admin", "mtk", "mld"}, template("admin_mtk/mtk_mld"), _("Mld configuration"), 2)
    entry({"admin", "mtk", "mld", "mld_cfg"}, call("mld_cfg")).leaf = true
    entry({"admin", "mtk", "mld", "mld_del"}, call("mld_del")).leaf = true
    entry({"admin", "mtk", "mld", "mld_edit"}, call("mld_edit")).leaf = true
    entry({"admin", "mtk", "mld", "reload_mld"}, call("reload_mld")).leaf = true
    entry({"admin", "mtk", "mld", "get_iface_list"}, call("get_iface_list")).leaf = true
    entry({"admin", "mtk", "console"}, template("admin_mtk/mtk_web_console"), _("Web Console"), 4)
    entry({"admin", "mtk", "webcmd"}, call("webcmd"))
    -- entry({"admin", "mtk", "man"}, template("admin_mtk/mtk_wifi_man"), _("M.A.N"), 3)
    -- entry({"admin", "mtk", "man", "cfg"}, call("man_cfg"))
    entry({"admin", "mtk", "wifi", "get_wps_info"}, call("get_WPS_Info")).leaf = true
    entry({"admin", "mtk", "wifi", "get_wifi_pin"}, call("get_wifi_pin")).leaf = true
    entry({"admin", "mtk", "wifi", "set_wifi_gen_pin"}, call("set_wifi_gen_pin")).leaf = true
    entry({"admin", "mtk", "wifi", "set_wifi_wps_oob"}, call("set_wifi_wps_oob")).leaf = true
    entry({"admin", "mtk", "wifi", "set_wifi_do_wps"}, call("set_wifi_do_wps")).leaf = true
    entry({"admin", "mtk", "wifi", "get_wps_security"}, call("get_wps_security")).leaf = true
    entry({"admin", "mtk", "wifi", "apcli_get_wps_status"}, call("apcli_get_wps_status")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_do_enr_pin_wps"}, call("apcli_do_enr_pin_wps")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_do_enr_pbc_wps"}, call("apcli_do_enr_pbc_wps")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_cancel_wps"}, call("apcli_cancel_wps")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_wps_gen_pincode"}, call("apcli_wps_gen_pincode")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_wps_get_pincode"}, call("apcli_wps_get_pincode")).leaf = true;
    entry({"admin", "mtk", "wifi", "apcli_scan"}, call("apcli_scan")).leaf = true;
    entry({"admin", "mtk", "wifi", "sta_info"}, call("sta_info")).leaf = true;
    entry({"admin", "mtk", "wifi", "get_apcli_conn_info"}, call("get_apcli_conn_info")).leaf = true;
    entry({"admin", "mtk", "wifi", "apply_power_boost_settings"}, call("apply_power_boost_settings")).leaf = true;
    entry({"admin", "mtk", "wifi", "apply_reboot"}, template("admin_mtk/mtk_wifi_apply_reboot")).leaf = true;
    entry({"admin", "mtk", "wifi", "reboot"}, call("exec_reboot")).leaf = true;
    entry({"admin", "mtk", "wifi", "get_bssid_num"}, call("get_bssid_num")).leaf = true;
    entry({"admin", "mtk", "wifi", "loading"}, template("admin_mtk/mtk_wifi_loading")).leaf = true;
    entry({"admin", "mtk", "wifi", "get_apply_status"}, call("get_apply_status")).leaf = true;
    entry({"admin", "mtk", "wifi", "reset_to_defaults"}, call("reset_to_defaults")).leaf = true;
    entry({"admin", "mtk", "wifi", "get_qosmgmt_rule_list"}, call("get_qosmgmt_rule_list")).leaf = true;
    entry({"admin", "mtk", "wifi", "qosmgmt_config_done"}, call("qosmgmt_config_done")).leaf = true;
    entry({"admin", "mtk", "wifi", "qosmgmt_config_change"}, call("qosmgmt_config_change")).leaf = true;

    local mtkwifi = require("mtkwifi")
    -- local profiles = mtkwifi.search_dev_and_profile()
    -- for devname,profile in pairs(profiles) do
    --     local cfgs = mtkwifi.load_profile(profile)
    --     if cfgs["VOW_Airtime_Fairness_En"] then
    --         entry({"admin", "mtk", "vow"}, template("admin_mtk/mtk_vow"), _("VoW / ATF / ATC"), 4)
    --         break
    --     end
    -- end

    -- Define map_help again here as same defination at top does not come under scope of luci library.
    local map_help
    if pcall(require, "map_helper") then
        map_help = require "map_helper"
    end
    if map_help then
        entry({"admin", "mtk", "multi_ap", "reset_to_default_easymesh"}, call("reset_to_default_easymesh")).leaf = true;
        entry({"admin", "mtk", "multi_ap"}, template("admin_mtk/mtk_wifi_multi_ap"), _("EasyMesh"), 5);
        entry({"admin", "mtk", "multi_ap", "map_cfg"}, call("map_cfg")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_device_role"}, call("get_device_role")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_mandate_steering_on_agent"}, call("trigger_mandate_steering_on_agent")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_back_haul_steering_on_agent"}, call("trigger_back_haul_steering_on_agent")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_wps_fh_agent"}, call("trigger_wps_fh_agent")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_runtime_topology"}, template("admin_mtk/mtk_wifi_map_runtime_topology")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_runtime_topology"}, call("get_runtime_topology")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_data_element"}, template("admin_mtk/mtk_wifi_map_data_element")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_channel_scan_result"}, template("admin_mtk/mtk_wifi_map_channel_scan_result")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_channel_planning_score"}, template("admin_mtk/mtk_wifi_map_channel_planning_score")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_multi_ap_on_boarding"}, call("trigger_multi_ap_on_boarding")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_client_capabilities"}, template("admin_mtk/mtk_wifi_map_client_capabilities")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_client_capabilities"}, call("get_client_capabilities")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_ap_capabilities"}, template("admin_mtk/mtk_wifi_map_ap_capabilities")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_uplink_ap_selection"}, call("trigger_uplink_ap_selection")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_bh_connection_status"}, call("get_bh_connection_status")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_sta_steering_progress"}, call("get_sta_steering_progress")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_al_mac"}, call("get_al_mac")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_wifi_bh_priority"}, call("apply_wifi_bh_priority")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_ap_steer_rssi_th"}, call("apply_ap_steer_rssi_th")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_channel_utilization_th"}, call("apply_channel_utilization_th")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_sta_bh_interface"}, call("get_sta_bh_interface")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_ap_bh_inf_list"}, call("get_ap_bh_inf_list")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_ap_fh_inf_list"}, call("get_ap_fh_inf_list")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_fh_status_bss"}, template("admin_mtk/mtk_wifi_map_bssinfo")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_bh_link_metrics_ctrler"}, template("admin_mtk/mtk_wifi_map_bh_link_metrics")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "easymesh_bss_config_renew"}, template("admin_mtk/mtk_wifi_map_bss_cfg_renew")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "easymesh_bss_cfg"}, call("easymesh_bss_cfg")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "validate_add_easymesh_bss_req"}, call("validate_add_easymesh_bss_req")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "remove_easymesh_bss_cfg_req"}, call("remove_easymesh_bss_cfg_req")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_easymesh_bss_cfg"}, call("apply_easymesh_bss_cfg")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "easymesh_bhInfo_config_renew"}, template("admin_mtk/mtk_wifi_map_bh_sta_cfg_renew")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "easymesh_sta_mlo_cfg"}, call("easymesh_sta_mlo_cfg")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "updata_easymesh_sta_mlo_cfg_req"}, call("updata_easymesh_sta_mlo_cfg_req")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_force_ch_switch"}, call("apply_force_ch_switch")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "apply_user_preferred_channel"}, call("apply_user_preferred_channel")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_channel_planning_r2"}, call("trigger_channel_planning_r2")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_de_dump"}, call("trigger_de_dump")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_data_element"}, call("get_data_element")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "trigger_channel_scan"}, call("trigger_channel_scan")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_channel_stats"}, call("get_channel_stats")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_channel_planning_score"}, call("get_channel_planning_score")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_user_preferred_channel"}, call("get_user_preferred_channel")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_sp_rule_list"}, call("get_sp_rule_list")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "del_sp_rule"}, call("del_sp_rule")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "sp_rule_reorder"}, call("sp_rule_reorder")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "sp_rule_move"}, call("sp_rule_move")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "add_sp_rule"}, call("sp_rule_add")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "sp_config_done"}, call("sp_config_done")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "get_qos_rule_list"}, call("get_qos_rule_list")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "qos_config_done"}, call("qos_config_done")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "qos_config_change"}, call("qos_config_change")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "submit_dpp_uri"}, call("submit_dpp_uri")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "display_bootstrapping_uri"}, template("admin_mtk/mtk_wifi_map_display_bootstrapping_uri")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "start_dpp_onboarding"}, call("start_dpp_onboarding")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "generate_dpp_uri"}, call("generate_dpp_uri")).leaf = true;
        entry({"admin", "mtk", "multi_ap", "retrive_dpp_uri"}, call("retrive_dpp_uri")).leaf = true;
    end
end

function test()
    http.write_json(http.formvalue())
end

function exec_reboot()
    os.execute("rm -f /tmp/mtk/wifi/reboot_required >/dev/null 2>&1")
    os.execute("sync >/dev/null 2>&1")
    os.execute("reboot >/dev/null 2>&1")
end

function get_apply_status()
    local ret = {}

    if mtkwifi.is_child_active() then
        ret["status"] = "ON_PROGRESS"
    elseif mtkwifi.exists("/tmp/mtk/wifi/reboot_required") then
        -- If the "wifi restart" command can not re-install the driver; then, it will create
        -- "/tmp/mtk/wifi/reboot_required" file to indicate LuCI that the settings will be applied
        -- only after reboot of the device.
        -- Redirect "Reboot Device" web-page to get consent from the user to reboot the device.
        ret["status"] = "REBOOT"
    else
        ret["status"] = "DONE"
    end
    http.write_json(ret)
end

function __mtkwifi_save_cfg(cfgs, devname, isProfileSettingsAppliedToDriver)
    local path = "/etc/config/wireless"
    if not mtkwifi.exists(mtkwifi.__uci_applied_settings_path()) then
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end
    if isProfileSettingsAppliedToDriver then
        -- It means the some context based profile settings to be saved in DAT file is already applied to the driver.
        -- Find the profile settings which are not applied to the driver before saving the new profile settings
        local diff = mtkwifi.diff_cfg(devname)
        mtkwifi.save_cfg(cfgs, devname)
        -- If there are any settings which are not applied to the driver, then do NOT copy and WebUI will display the "need reload to apply changes" message
        -- Otherwise, copy the new profile settings and WebUI will NOT display the "need reload to apply changes" message
        if next(diff) == nil then
            os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
        end
    else
        mtkwifi.save_cfg(cfgs, devname)
    end
end

local __mtkwifi_restart = function (devname)
    os.execute("wifi restart "..(devname or ""))
    debug_write("wifi restart "..(devname or ""))

    if devname then
        local cfg = mtkwifi.load_cfg(devname, "", true)
        mtkwifi.save_cfg(cfg, devname, mtkwifi.__uci_applied_config())
    else
        local path = "/etc/config/wireless"
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end
end

local __mtkwifi_reload = function (devname, is_diff)
    local wifi_restart = false
    local wifi_reload = false

    for _, dev in pairs(ucicfg["wifi-device"]) do
        local devN = string.gsub(dev[".name"], "%_", ".")
        if not devname or devname == devN then
            local diff = mtkwifi.diff_cfg(devN, "", true)
            __process_settings_before_apply(devN, diff)
            flag = false
            for k,v in pairs(diff) do
                if k == "ApEnable" then
                    cfg1 = diff["ApEnable"][1]:split(";")
                    cfg2 = diff["ApEnable"][2]:split(";")
                    for i=1, #cfg1 do
                        if cfg1[i] ~= cfg2[i] and cfg1[i] == "0" then
                            flag = true
                        end
                    end
                end
            end
            if diff.BssidNum or diff.WHNAT or diff.E2pAccessMode or diff.HE_LDPC or diff.WdsEnable then
                -- Addition or deletion of a vif requires re-installation of the driver.
                -- Change in WHNAT setting also requires re-installation of the driver.
                -- Driver will be re-installed by "wifi restart" command.
                wifi_restart = true
            elseif flag == true then
                wifi_restart = true
            else
                wifi_reload = true
            end

        end
    end

    if wifi_restart then
        os.execute("wifi restart "..(devname or ""))
        debug_write("wifi restart "..(devname or ""))
    elseif wifi_reload then
        os.execute("wifi reload "..(devname or ""))
        debug_write("wifi reload "..(devname or ""))
    end

    if devname then
        local cfg = mtkwifi.load_cfg(devname, "", true)
        mtkwifi.save_cfg(cfg, devname, mtkwifi.__uci_applied_config())
    else
        local path = "/etc/config/wireless"
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end

    if map_help then
        local easymesh_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__read_easymesh_profile_path())
        os.execute("cp -f "..mtkwifi.__read_easymesh_profile_path().." "..easymesh_applied_path)
    end
end

function __process_settings_before_apply(devname, diff)
    local devs = mtkwifi.get_all_devs()
    local cfgs = mtkwifi.load_cfg(devname, "", true)
    __apply_wifi_wpsconf(devs, devname, cfgs, diff)
end

function chip_modify(devname, httpObj)
    local cfgs = mtkwifi.load_cfg(devname, "", true)

    for k,v in pairs(httpObj) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "chip_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "chip_cfg, special: "..k.."="..v)
        else
            cfgs[k] = v or ""
        end
    end

    -- VOW
    -- ATC should actually be scattered into each SSID, but I'm just lazy.
    if cfgs.VOW_Airtime_Fairness_En then
        for i = 1,tonumber(cfgs.BssidNum) do
            __atc_tp     = http.formvalue("__atc_vif"..i.."_tp")     or "0"
            __atc_min_tp = http.formvalue("__atc_vif"..i.."_min_tp") or "0"
            __atc_max_tp = http.formvalue("__atc_vif"..i.."_max_tp") or "0"
            __atc_at     = http.formvalue("__atc_vif"..i.."_at")     or "0"
            __atc_min_at = http.formvalue("__atc_vif"..i.."_min_at") or "0"
            __atc_max_at = http.formvalue("__atc_vif"..i.."_max_at") or "0"

            nixio.syslog("info", "ATC.__atc_tp     ="..i..__atc_tp     );
            nixio.syslog("info", "ATC.__atc_min_tp ="..i..__atc_min_tp );
            nixio.syslog("info", "ATC.__atc_max_tp ="..i..__atc_max_tp );
            nixio.syslog("info", "ATC.__atc_at     ="..i..__atc_at     );
            nixio.syslog("info", "ATC.__atc_min_at ="..i..__atc_min_at );
            nixio.syslog("info", "ATC.__atc_max_at ="..i..__atc_max_at );

            cfgs.VOW_Rate_Ctrl_En    = mtkwifi.token_set(cfgs.VOW_Rate_Ctrl_En,    i, __atc_tp)
            cfgs.VOW_Group_Min_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Min_Rate,  i, __atc_min_tp)
            cfgs.VOW_Group_Max_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Max_Rate,  i, __atc_max_tp)

            cfgs.VOW_Airtime_Ctrl_En = mtkwifi.token_set(cfgs.VOW_Airtime_Ctrl_En, i, __atc_at)
            cfgs.VOW_Group_Min_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Min_Ratio, i, __atc_min_at)
            cfgs.VOW_Group_Max_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Max_Ratio, i, __atc_max_at)

        end

        cfgs.VOW_RX_En = http.formvalue("VOW_RX_En") or "0"
    end
    return cfgs
end

function chip_cfg(devname)
    local devs = mtkwifi.get_all_devs()
    local dev = {}
    dev = devs and devs[devname]
    local cfgs = {}

    if dev.dbdc == true then
        for _, dev in pairs(ucicfg["wifi-device"]) do
            local devname = string.gsub(dev[".name"], "%_", ".")
            cfgs = chip_modify(devname, http.formvalue())
            __mtkwifi_save_cfg(cfgs, devname)
        end
    else
        cfgs = chip_modify(devname, http.formvalue())
        __mtkwifi_save_cfg(cfgs, devname)
    end

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "wifi", "chip_cfg_view",devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "chip_cfg_view",devname))
    end
end



function dev_cfg(devname)
    local cfgs = mtkwifi.load_cfg(devname, "", true)

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "dev_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "dev_cfg, special: "..k.."="..v)
        else
            cfgs[k] = v or ""
        end
    end

    if cfgs.Channel == "0" then -- Auto Channel Select
        cfgs.AutoChannelSelect = "3"
    else
        cfgs.AutoChannelSelect = "0"
    end

    if http.formvalue("__bw") == "20" then
        cfgs.HT_BW = '0'
        cfgs.VHT_BW = '0'
        cfgs.EHT_ApBw = '0'
    elseif http.formvalue("__bw") == "40" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '0'
        cfgs.EHT_ApBw = '1'
    elseif http.formvalue("__bw") == "60" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '0'
    elseif http.formvalue("__bw") == "80" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '1'
        cfgs.EHT_ApBw = '2'
    elseif http.formvalue("__bw") == "160" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '2'
        cfgs.EHT_ApBw = '3'
    elseif http.formvalue("__bw") == "161" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '3'
        cfgs.VHT_Sec80_Channel = http.formvalue("VHT_Sec80_Channel") or ""
    elseif http.formvalue("__bw") == "320" then
        cfgs.HT_BW = '1'
        cfgs.VHT_BW = '2'
        cfgs.EHT_ApBw = '4'
    end

    if mtkwifi.band(string.split(cfgs.WirelessMode,";")[1]) == "5G" or mtkwifi.band(cfgs.WirelessMode) == "6G" then
        cfgs.CountryRegionABand = http.formvalue("__cr");
    else
        cfgs.CountryRegion = http.formvalue("__cr");
    end

    if http.formvalue("TxPower") then
        local txpower = tonumber(http.formvalue("TxPower"))
        if txpower <= 100 then
            cfgs.PERCENTAGEenable=1
        else
            cfgs.PERCENTAGEenable=0
        end
    end

    local IndividualTWTSupport = tonumber(http.formvalue("IndividualTWTSupport"))
    if IndividualTWTSupport == 0 then
        cfgs.TWTResponder=0
        cfgs.TWTRequired=0
    elseif IndividualTWTSupport == 1 then
        cfgs.TWTResponder=1
        cfgs.TWTRequired=0
    else
        cfgs.TWTResponder=1
        cfgs.TWTRequired=1
    end

    local mimo = http.formvalue("__mimo")
    if mimo == "0" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=0
    elseif mimo == "1" then
        cfgs.ETxBfEnCond=0
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "2" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "3" then
        cfgs.ETxBfEnCond=1
        if tonumber(cfgs.ApCliEnable) == 1 then
            cfgs.MUTxRxEnable=3
        else
            cfgs.MUTxRxEnable=1
        end
        cfgs.ITxBfEn=0
    elseif mimo == "4" then
        cfgs.ETxBfEnCond=1
        if tonumber(cfgs.ApCliEnable) == 1 then
            cfgs.MUTxRxEnable=3
        else
            cfgs.MUTxRxEnable=1
        end
        cfgs.ITxBfEn=1
    else
        cfgs.ETxBfEnCond=0
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=0
    end

--    if cfgs.ApCliEnable == "1" then
--        cfgs.Channel = http.formvalue("__apcli_channel")
--    end

    -- WDS
    -- http.write_json(http.formvalue())
    __mtkwifi_save_cfg(cfgs, devname)

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "wifi", "dev_cfg_view",devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "dev_cfg_view",devname))
    end
end



function __delete_mbss_para(cfgs, vif_idx)
    debug_write(vif_idx)
    cfgs["WPAPSK"..vif_idx]=""
    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vif_idx,"")
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vif_idx,"")
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vif_idx,"")
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vif_idx,"")
    cfgs["RADIUS_Server"]=mtkwifi.token_set(cfgs["RADIUS_Server"],vif_idx,"")
    cfgs["RADIUS_Port"]=mtkwifi.token_set(cfgs["RADIUS_Port"],vif_idx,"")
    cfgs["RADIUS_Key"..vif_idx]=""
    cfgs["DefaultKeyID"]=mtkwifi.token_set(cfgs["DefaultKeyID"],vif_idx,"")
    cfgs["IEEE8021X"]=mtkwifi.token_set(cfgs["IEEE8021X"],vif_idx,"")
    cfgs["WscConfMode"]=mtkwifi.token_set(cfgs["WscConfMode"],vif_idx,"")
    cfgs["PreAuth"]=mtkwifi.token_set(cfgs["PreAuth"],vif_idx,"")
    cfgs["HT_STBC"] = mtkwifi.token_set(cfgs["HT_STBC"],vif_idx,"")
    cfgs["HT_LDPC"] = mtkwifi.token_set(cfgs["HT_LDPC"],vif_idx,"")
    cfgs["VHT_STBC"] = mtkwifi.token_set(cfgs["VHT_STBC"],vif_idx,"")
    cfgs["VHT_LDPC"] = mtkwifi.token_set(cfgs["VHT_LDPC"],vif_idx,"")
    cfgs["HideSSID"]=mtkwifi.token_set(cfgs["HideSSID"],vif_idx,"")
    cfgs["NoForwarding"]=mtkwifi.token_set(cfgs["NoForwarding"],vif_idx,"")
    cfgs["WmmCapable"]=mtkwifi.token_set(cfgs["WmmCapable"],vif_idx,"")
    cfgs["TxRate"]=mtkwifi.token_set(cfgs["TxRate"],vif_idx,"")
    cfgs["RekeyInterval"]=mtkwifi.token_set(cfgs["RekeyInterval"],vif_idx,"")
    cfgs["AuthMode"]=mtkwifi.token_set(cfgs["AuthMode"],vif_idx,"")
    cfgs["EncrypType"]=mtkwifi.token_set(cfgs["EncrypType"],vif_idx,"")
    cfgs["session_timeout_interval"]=mtkwifi.token_set(cfgs["session_timeout_interval"],vif_idx,"")
    cfgs["WscModeOption"]=mtkwifi.token_set(cfgs["WscModeOption"],vif_idx,"")
    cfgs["RekeyMethod"]=mtkwifi.token_set(cfgs["RekeyMethod"],vif_idx,"")
    cfgs["PMFMFPC"] = mtkwifi.token_set(cfgs["PMFMFPC"],vif_idx,"")
    cfgs["PMFMFPR"] = mtkwifi.token_set(cfgs["PMFMFPR"],vif_idx,"")
    cfgs["PMFSHA256"] = mtkwifi.token_set(cfgs["PMFSHA256"],vif_idx,"")
    cfgs["PMKCachePeriod"] = mtkwifi.token_set(cfgs["PMKCachePeriod"],vif_idx,"")
    cfgs["Wapiifname"] = mtkwifi.token_set(cfgs["Wapiifname"],vif_idx,"")
    cfgs["RRMEnable"] = mtkwifi.token_set(cfgs["RRMEnable"],vif_idx,"")
    cfgs["DLSCapable"] = mtkwifi.token_set(cfgs["DLSCapable"],vif_idx,"")
    cfgs["APSDCapable"] = mtkwifi.token_set(cfgs["APSDCapable"],vif_idx,"")
    cfgs["FragThreshold"] = mtkwifi.token_set(cfgs["FragThreshold"],vif_idx,"")
    cfgs["RTSThreshold"] = mtkwifi.token_set(cfgs["RTSThreshold"],vif_idx,"")
    cfgs["VHT_SGI"] = mtkwifi.token_set(cfgs["VHT_SGI"],vif_idx,"")
    cfgs["VHT_BW_SIGNAL"] = mtkwifi.token_set(cfgs["VHT_BW_SIGNAL"],vif_idx,"")
    cfgs["HT_PROTECT"] = mtkwifi.token_set(cfgs["HT_PROTECT"],vif_idx,"")
    cfgs["HT_GI"] = mtkwifi.token_set(cfgs["HT_GI"],vif_idx,"")
    cfgs["HT_OpMode"] = mtkwifi.token_set(cfgs["HT_OpMode"],vif_idx,"")
    cfgs["HT_TxStream"] = mtkwifi.token_set(cfgs["HT_TxStream"],vif_idx,"")
    cfgs["HT_RxStream"] = mtkwifi.token_set(cfgs["HT_RxStream"],vif_idx,"")
    cfgs["HT_AMSDU"] = mtkwifi.token_set(cfgs["HT_AMSDU"],vif_idx,"")
    cfgs["HT_AutoBA"] = mtkwifi.token_set(cfgs["HT_AutoBA"],vif_idx,"")
    cfgs["HT_BAWinSize"] = mtkwifi.token_set(cfgs["HT_BAWinSize"],vif_idx,"")
    cfgs["HT_BADecline"] = mtkwifi.token_set(cfgs["HT_BADecline"],vif_idx,"")
    cfgs["IgmpSnEnable"] = mtkwifi.token_set(cfgs["IgmpSnEnable"],vif_idx,"")
    cfgs["WirelessMode"] = mtkwifi.token_set(cfgs["WirelessMode"],vif_idx,"")
    cfgs["MldGroup"] = mtkwifi.token_set(cfgs["MldGroup"],vif_idx,"")
    cfgs["WdsEnable"] = mtkwifi.token_set(cfgs["WdsEnable"],vif_idx,"")
    cfgs["MuOfdmaDlEnable"] = mtkwifi.token_set(cfgs["MuOfdmaDlEnable"],vif_idx,"")
    cfgs["MuOfdmaUlEnable"] = mtkwifi.token_set(cfgs["MuOfdmaUlEnable"],vif_idx,"")
    cfgs["MuMimoDlEnable"] = mtkwifi.token_set(cfgs["MuMimoDlEnable"],vif_idx,"")
    cfgs["MuMimoUlEnable"] = mtkwifi.token_set(cfgs["MuMimoUlEnable"],vif_idx,"")
    cfgs["DtimPeriod"] = mtkwifi.token_set(cfgs["DtimPeriod"],vif_idx,"")
    cfgs["Mrsno_En"] = mtkwifi.token_set(cfgs["Mrsno_En"],vif_idx,"")
end

function vif_del(dev, vif)
    local path = "/etc/config/wireless"
    if not mtkwifi.exists(mtkwifi.__uci_applied_settings_path()) then
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end

    debug_write("vif_del("..dev..vif..")")
    local devname,vifname = dev, vif
    debug_write("devname="..devname)
    debug_write("vifname="..vifname)
    local devs = mtkwifi.get_all_devs()
    local idx = devs[devname]["vifs"][vifname].vifidx -- or tonumber(string.match(vifname, "%d+")) + 1
    debug_write("idx="..idx, devname, vifname)
    local pattern = "%d+"
    local ext_vifname = string.gsub(vifname, pattern, "")
    if idx and tonumber(idx) >= 0 then
        local cfgs = mtkwifi.load_cfg(devname, "wireless", true)
        BssidNum = tonumber(cfgs["BssidNum"])
        idx_n = tonumber(idx)
        if cfgs then
            os.execute("uci delete wireless."..vifname)
            if BssidNum > idx_n then
                for id=1, BssidNum-idx_n do
                    index = idx_n+id-1
                    os.execute("uci rename wireless."..ext_vifname..index.."="..ext_vifname..index-1)
                end
            end
            for _,mld in ipairs(cfgs.mld) do
                if mld.iface == vifname then
                    os.execute("uci delete wireless."..mld["name"])
                else
                    mld_list = string.split(mld.iface, " ")
                    for i,m in ipairs(mld_list) do
                        if m == vifname then
                            table.remove(mld_list, i)
                        end
                        num = tonumber(string.match(m, "%d+"))
                        ext_name = string.gsub(m, pattern, "")
                        if (num > idx_n-1) and (ext_name == ext_vifname) then
                            mld_list[i] = ext_vifname..(num-1)
                        end
                    end
                    new_str = table.concat(mld_list, " ")
                    os.execute("uci set wireless."..mld["name"]..".iface='"..new_str.."'")
                end
            end
            os.execute("uci commit")
        else
            debug_write(devname.." cannot be found!")
        end
    end
    os.execute("ifconfig "..vif.." down")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
end

function vif_disable(iface)
    os.execute("ifconfig "..iface.." down")
    os.execute("uci set wireless."..iface..".disabled=1")
    os.execute("uci commit")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
end

function vif_enable(iface)
    os.execute("ifconfig "..iface.." up")
    os.execute("uci set wireless."..iface..".disabled=0")
    os.execute("uci commit")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
end


--[[
-- security config in mtk wifi is quite complicated!
-- cfgs listed below are attached with vif and combined like "0;0;0;0". They need specicial treatment.
        TxRate, WmmCapable, NoForwarding,
        HideSSID, IEEE8021X, PreAuth,
        AuthMode, EncrypType, RekeyMethod,
        RekeyInterval, PMKCachePeriod,
        DefaultKeyId, Key{n}Type, HT_EXTCHA,
        RADIUS_Server, RADIUS_Port,
]]

local function conf_wep_keys(cfgs,vifidx)
    cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vifidx, http.formvalue("__DefaultKeyID") or 1)
    cfgs["Key1Str"..vifidx]  = http.formvalue("Key1Str"..vifidx)
    cfgs["Key2Str"..vifidx]  = http.formvalue("Key2Str"..vifidx)
    cfgs["Key3Str"..vifidx]  = http.formvalue("Key3Str"..vifidx)
    cfgs["Key4Str"..vifidx]  = http.formvalue("Key4Str"..vifidx)

    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vifidx, http.formvalue("WEP1Type"..vifidx))
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vifidx, http.formvalue("WEP2Type"..vifidx))
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vifidx, http.formvalue("WEP3Type"..vifidx))
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vifidx, http.formvalue("WEP4Type"..vifidx))

    return cfgs
end

local function __security_cfg(cfgs, vif_idx)
    debug_write("__security_cfg, before, HideSSID="..tostring(cfgs.HideSSID))
    debug_write("__security_cfg, before, NoForwarding="..tostring(cfgs.NoForwarding))
    debug_write("__security_cfg, before, WmmCapable="..tostring(cfgs.WmmCapable))
    debug_write("__security_cfg, before, TxRate="..tostring(cfgs.TxRate))
    debug_write("__security_cfg, before, RekeyInterval="..tostring(cfgs.RekeyInterval))
    debug_write("__security_cfg, before, AuthMode="..tostring(cfgs.AuthMode))
    debug_write("__security_cfg, before, EncrypType="..tostring(cfgs.EncrypType))
    debug_write("__security_cfg, before, WscModeOption="..tostring(cfgs.WscModeOption))
    debug_write("__security_cfg, before, RekeyMethod="..tostring(cfgs.RekeyMethod))
    debug_write("__security_cfg, before, IEEE8021X="..tostring(cfgs.IEEE8021X))
    debug_write("__security_cfg, before, DefaultKeyID="..tostring(cfgs.DefaultKeyID))
    debug_write("__security_cfg, before, PMFMFPC="..tostring(cfgs.PMFMFPC))
    debug_write("__security_cfg, before, PMFMFPR="..tostring(cfgs.PMFMFPR))
    debug_write("__security_cfg, before, PMFSHA256="..tostring(cfgs.PMFSHA256))
    debug_write("__security_cfg, before, RADIUS_Server="..tostring(cfgs.RADIUS_Server))
    debug_write("__security_cfg, before, RADIUS_Port="..tostring(cfgs.RADIUS_Port))
    debug_write("__security_cfg, before, session_timeout_interval="..tostring(cfgs.session_timeout_interval))
    debug_write("__security_cfg, before, PMKCachePeriod="..tostring(cfgs.PMKCachePeriod))
    debug_write("__security_cfg, before, PreAuth="..tostring(cfgs.PreAuth))
    debug_write("__security_cfg, before, Wapiifname="..tostring(cfgs.Wapiifname))

    -- Reset/Clear all necessary settings here. Later, these settings will be set as per AuthMode.
    cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "DISABLE")
    cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
    cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "0")
    cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "0")
    cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    -- Update the settings which are not dependent on AuthMode
    cfgs.HideSSID = mtkwifi.token_set(cfgs.HideSSID, vif_idx, http.formvalue("__hidessid") or "0")
    cfgs.NoForwarding = mtkwifi.token_set(cfgs.NoForwarding, vif_idx, http.formvalue("__noforwarding") or "0")
    cfgs.WmmCapable = mtkwifi.token_set(cfgs.WmmCapable, vif_idx, http.formvalue("__wmmcapable") or "0")
    cfgs.TxRate = mtkwifi.token_set(cfgs.TxRate, vif_idx, http.formvalue("__txrate") or "0");
    cfgs.RekeyInterval = mtkwifi.token_set(cfgs.RekeyInterval, vif_idx, http.formvalue("__rekeyinterval") or "0");

    local __authmode = http.formvalue("__authmode") or "Disable"
    cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, __authmode)

    if __authmode == "Disable" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "NONE")
    -- remove WEP mode
    -- elseif __authmode == "OPEN" or __authmode == "SHARED" or __authmode == "WEPAUTO" then
        -- cfgs.WscModeOption = "0"
        -- cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "WEP")
        -- cfgs = conf_wep_keys(cfgs,vif_idx)

    elseif __authmode == "Enhanced Open" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OWE")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPAPSK"  then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")

    elseif __authmode == "WPAPSKWPA2PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.WpaMixPairCipher = "WPA_TKIP_WPA2_AES"

    elseif __authmode == "WPA2PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")

    elseif __authmode == "WPA3PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "NONE")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA2PSKWPA3PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "NONE")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA2" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")

    elseif __authmode == "WPA3" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA3-192-bit" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "WPA3-192")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "GCMP256")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA1WPA2" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "1812")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
    elseif __authmode == "WPA3PSKCompt" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "NONE")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "IEEE8021X" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__8021x_wep") and "WEP" or "NONE")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "1")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")

    elseif __authmode == "WAICERT" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        cfgs.Wapiifname = mtkwifi.token_set(cfgs.Wapiifname, vif_idx, "br-lan")
        -- cfgs.wapicert_asipaddr
        -- cfgs.WapiAsPort
        -- cfgs.wapicert_ascert
        -- cfgs.wapicert_usercert

    elseif __authmode == "WAIPSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        -- cfgs.wapipsk_keytype
        -- cfgs.wapipsk_prekey
    end

    debug_write("__security_cfg, after, HideSSID="..tostring(cfgs.HideSSID))
    debug_write("__security_cfg, after, NoForwarding="..tostring(cfgs.NoForwarding))
    debug_write("__security_cfg, after, WmmCapable="..tostring(cfgs.WmmCapable))
    debug_write("__security_cfg, after, TxRate="..tostring(cfgs.TxRate))
    debug_write("__security_cfg, after, RekeyInterval="..tostring(cfgs.RekeyInterval))
    debug_write("__security_cfg, after, AuthMode="..tostring(cfgs.AuthMode))
    debug_write("__security_cfg, after, EncrypType="..tostring(cfgs.EncrypType))
    debug_write("__security_cfg, after, WscModeOption="..tostring(cfgs.WscModeOption))
    debug_write("__security_cfg, after, RekeyMethod="..tostring(cfgs.RekeyMethod))
    debug_write("__security_cfg, after, IEEE8021X="..tostring(cfgs.IEEE8021X))
    debug_write("__security_cfg, after, DefaultKeyID="..tostring(cfgs.DefaultKeyID))
    debug_write("__security_cfg, after, PMFMFPC="..tostring(cfgs.PMFMFPC))
    debug_write("__security_cfg, after, PMFMFPR="..tostring(cfgs.PMFMFPR))
    debug_write("__security_cfg, after, PMFSHA256="..tostring(cfgs.PMFSHA256))
    debug_write("__security_cfg, after, RADIUS_Server="..tostring(cfgs.RADIUS_Server))
    debug_write("__security_cfg, after, RADIUS_Port="..tostring(cfgs.RADIUS_Port))
    debug_write("__security_cfg, after, session_timeout_interval="..tostring(cfgs.session_timeout_interval))
    debug_write("__security_cfg, after, PMKCachePeriod="..tostring(cfgs.PMKCachePeriod))
    debug_write("__security_cfg, after, PreAuth="..tostring(cfgs.PreAuth))
    debug_write("__security_cfg, after, Wapiifname="..tostring(cfgs.Wapiifname))
end

function initialize_multiBssParameters(cfgs,vif_idx)
    cfgs["WPAPSK"..vif_idx]="12345678"
    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vif_idx,"0")
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vif_idx,"0")
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vif_idx,"0")
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vif_idx,"0")
    cfgs["RADIUS_Server"]=mtkwifi.token_set(cfgs["RADIUS_Server"],vif_idx,"0")
    cfgs["RADIUS_Port"]=mtkwifi.token_set(cfgs["RADIUS_Port"],vif_idx,"1812")
    cfgs["RADIUS_Key"..vif_idx]="ralink"
    cfgs["DefaultKeyID"]=mtkwifi.token_set(cfgs["DefaultKeyID"],vif_idx,"1")
    cfgs["IEEE8021X"]=mtkwifi.token_set(cfgs["IEEE8021X"],vif_idx,"0")
    cfgs["WscConfMode"]=mtkwifi.token_set(cfgs["WscConfMode"],vif_idx,"0")
    cfgs["PreAuth"]=mtkwifi.token_set(cfgs["PreAuth"],vif_idx,"0")
    return cfgs
end

function __wps_ap_pbc_start_all(ifname)
    os.execute("hostapd_cli -i "..ifname.." wps_pbc")
end

function __wps_ap_pin_start_all(ifname, pincode)
    os.execute("hostapd_cli -i "..ifname.." wps_pin any "..pincode)
end

local __restart_miniupnpd = function (devName,ifName)
    if pcall(require, "wifi_services") then
        -- OpenWRT
        assert(type(devName) == type(""))
        assert(type(ifName) == type(""))
        local wifi_service = require("wifi_services")
        --debug_write("Call miniupnpd_chk() of wifi_services module")
        --miniupnpd_chk(devName,ifName,wifi_service)
    else
        -- LSDK
        debug_write("Execute miniupnpd.sh script!")
        os.execute("miniupnpd.sh init")
    end
end

local __restart_hotspot_daemon = function ()
    os.execute("killall hs")
    os.execute("rm -rf /tmp/hotspot*")
    -- As this function is executed in child environment, there is no need to spawn it using fork-exec method.
    os.execute("hs -d 1 -v 2 -f/etc_ro/hotspot_ap.conf")
end

local __restart_8021x = function (devName,ifName)
    if pcall(require, "wifi_services") then
        -- OpenWRT
        assert(type(devName) == type(""))
        assert(type(ifName) == type(""))
        local ifPrefix = string.match(ifName,"([a-z]+)")
        assert(type(ifPrefix) == type(""))
        local wifi_service = require("wifi_services")
        debug_write("Call d8021xd_chk() of wifi_services module")
        d8021xd_chk(devName,ifPrefix,ifPrefix.."0",true)
    else
        -- LSDK
        debug_write("Call mtkwifi.restart_8021x()")
        mtkwifi.restart_8021x(devName)
    end
end

--Landen: CP functions from wireless for Ajax, reloading page is not required when DBDC ssid changed.
local __restart_all_daemons = function (devName,ifName)
    --__restart_8021x(devName,ifName)
    __restart_hotspot_daemon()
    --__restart_miniupnpd(devName,ifName)
end

function __apply_wifi_wpsconf(devs, devname, cfgs, diff)
    local saved = cfgs.WscConfMode and cfgs.WscConfMode:gsub(";-(%d);-","%1") or ""
    local applied = diff.WscConfMode and diff["WscConfMode"][2]:gsub(";-(%d);-","%1") or ""
    local num_ifs = tonumber(cfgs.BssidNum) or 0

    for idx=1, num_ifs do
        local ifname = devs[devname]["vifs"][idx]["vifname"]
        if mtkwifi.__any_wsc_enabled(saved:sub(idx,idx) == 1) then
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, idx, "2")
        else
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, idx, "1")
        end
        if (diff.WscConfMode) and saved:sub(idx,idx) ~= applied:sub(idx,idx) then
            cfgs = mtkwifi.__restart_if_wps(devname, ifname, cfgs)
        end
    end

    __mtkwifi_save_cfg(cfgs, devname)

    if diff.WscConfMode then
        for idx=1, num_ifs do
            local ifname = devs[devname]["vifs"][idx]["vifname"]
            if saved:sub(idx,idx) ~= applied:sub(idx,idx) then
                -- __restart_miniupnpd(devname, ifname)
            end
        end
    end
end

function __set_wifi_wpsconf(cfgs, wsc_enable, vif_idx)
    debug_write("__set_wifi_wpsconf : wsc_enable = ",wsc_enable)
    if(wsc_enable == "1") then
        cfgs["WscConfMode"] = mtkwifi.token_set(cfgs["WscConfMode"], vif_idx, "7")
    else
        cfgs["WscConfMode"] = mtkwifi.token_set(cfgs["WscConfMode"], vif_idx, "0")
    end
    if(((http.formvalue("__authmode")=="OPEN") and
        (http.formvalue("__encrypttype") == "WEP")) or
       (http.formvalue("__hidessid") == "1")) then
        cfgs.WscConfMode = mtkwifi.token_set(cfgs.WscConfMode, vif_idx, "0")
    end
    debug_write("__set_wifi_wpsconf : WscConfMode = ",cfgs["WscConfMode"])
end

function __update_mbss_para(cfgs, vif_idx)
    debug_write(vif_idx)
    cfgs.HT_STBC = mtkwifi.token_set(cfgs.HT_STBC, vif_idx, http.formvalue("__ht_stbc") or "0")
    cfgs.HT_LDPC = mtkwifi.token_set(cfgs.HT_LDPC, vif_idx, http.formvalue("__ht_ldpc") or "0")
    cfgs.VHT_STBC = mtkwifi.token_set(cfgs.VHT_STBC, vif_idx, http.formvalue("__vht_stbc") or "0")
    cfgs.VHT_LDPC = mtkwifi.token_set(cfgs.VHT_LDPC, vif_idx, http.formvalue("__vht_ldpc") or "0")
    --cfgs.DLSCapable = mtkwifi.token_set(cfgs.DLSCapable, vif_idx, http.formvalue("__dls_capable") or "0")
    cfgs.APSDCapable = mtkwifi.token_set(cfgs.APSDCapable, vif_idx, http.formvalue("__apsd_capable") or "0")
    cfgs.FragThreshold = mtkwifi.token_set(cfgs.FragThreshold, vif_idx, http.formvalue("__frag_threshold") or "0")
    cfgs.RTSThreshold = mtkwifi.token_set(cfgs.RTSThreshold, vif_idx, http.formvalue("__rts_threshold") or "0")
    cfgs.VHT_SGI = mtkwifi.token_set(cfgs.VHT_SGI, vif_idx, http.formvalue("__vht_sgi") or "0")
    cfgs.VHT_BW_SIGNAL = mtkwifi.token_set(cfgs.VHT_BW_SIGNAL, vif_idx, http.formvalue("__vht_bw_signal") or "0")
    cfgs.HT_PROTECT = mtkwifi.token_set(cfgs.HT_PROTECT, vif_idx, http.formvalue("__ht_protect") or "0")
    cfgs.HT_GI = mtkwifi.token_set(cfgs.HT_GI, vif_idx, http.formvalue("__ht_gi") or "0")
    cfgs.HT_OpMode = mtkwifi.token_set(cfgs.HT_OpMode, vif_idx, http.formvalue("__ht_opmode") or "0")
    cfgs.HT_AMSDU = mtkwifi.token_set(cfgs.HT_AMSDU, vif_idx, http.formvalue("__ht_amsdu") or "0")
    cfgs.HT_AutoBA = mtkwifi.token_set(cfgs.HT_AutoBA, vif_idx, http.formvalue("__ht_autoba") or "0")
    cfgs.HT_BAWinSize = mtkwifi.token_set(cfgs.HT_BAWinSize, vif_idx, http.formvalue("__ht_bawinsize") or mtkwifi.get_bawinsize(string.split(cfgs.WirelessMode,";")[1]))
    cfgs.HT_BADecline = mtkwifi.token_set(cfgs.HT_BADecline, vif_idx, http.formvalue("__ht_badecline") or "0")
    cfgs.IgmpSnEnable = mtkwifi.token_set(cfgs.IgmpSnEnable, vif_idx, http.formvalue("__igmp_snenable") or "0")
    cfgs.WirelessMode = mtkwifi.token_set(cfgs.WirelessMode, vif_idx, http.formvalue("__wirelessmode") or "0")
    cfgs.WdsEnable = mtkwifi.token_set(cfgs.WdsEnable, vif_idx, http.formvalue("__wdsenable") or "0")
    cfgs.MuOfdmaDlEnable = mtkwifi.token_set(cfgs.MuOfdmaDlEnable, vif_idx, http.formvalue("__muofdma_dlenable") or "0")
    cfgs.MuOfdmaUlEnable = mtkwifi.token_set(cfgs.MuOfdmaUlEnable, vif_idx, http.formvalue("__muofdma_ulenable") or "0")
    cfgs.MuMimoDlEnable = mtkwifi.token_set(cfgs.MuMimoDlEnable, vif_idx, http.formvalue("__mumimo_dlenable") or "0")
    cfgs.MuMimoUlEnable = mtkwifi.token_set(cfgs.MuMimoUlEnable, vif_idx, http.formvalue("__mumimo_ulenable") or "0")
    cfgs.DtimPeriod = mtkwifi.token_set(cfgs.DtimPeriod, vif_idx, http.formvalue("__dtim_period") or "1")
    cfgs.ApEnable = mtkwifi.token_set(cfgs.ApEnable, vif_idx, http.formvalue("__disable") or "0")
    cfgs.Mrsno_En = mtkwifi.token_set(cfgs.Mrsno_En, vif_idx, http.formvalue("__mrsno") or "0")
end

function vif_cfg(dev, vif)
    local devname, vifname = dev, vif
    if not devname then devname = vif end
    debug_write("devname="..devname)
    debug_write("vifname="..(vifname or ""))
    local devs = mtkwifi.get_all_devs()

    local cfgs = mtkwifi.load_cfg(devname, "wireless", true)

    for k,v in pairs(http.formvalue()) do
        if type(v) == type("") or type(v) == type(0) then
            nixio.syslog("debug", "post."..k.."="..tostring(v))
        else
            nixio.syslog("debug", "post."..k.." invalid, type="..type(v))
        end
    end

    -- sometimes vif_idx start from 0, like AccessPolicy0
    -- sometimes it starts from 1, like WPAPSK1. nice!
    local vif_idx
    local to_url
    if http.formvalue("__action") == "vif_cfg_view" then
        vif_idx = devs[devname]["vifs"][vifname].vifidx
        debug_write("vif_idx=", vif_idx, devname, vifname)
        to_url = luci.dispatcher.build_url("admin", "mtk", "wifi", "vif_cfg_view", devname, vifname)
    elseif http.formvalue("__action") == "vif_add_view" then
        cfgs.BssidNum = tonumber(cfgs.BssidNum) + 1
        vif_idx = tonumber(cfgs.BssidNum)
        to_url = luci.dispatcher.build_url("admin", "mtk", "wifi")
        -- initializing ; separated parameters for the new interface
        cfgs = initialize_multiBssParameters(cfgs, vif_idx)
    end
    assert(vif_idx)
    assert(to_url)
    -- "__" should not be the prefix of a name if user wants to copy form value data directly to the dat file variable
    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "vif_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            debug_write("vif_cfg: Copying",k,v)
            cfgs[k] = v or ""
        end
    end

    -- WDS
    -- Update WdsXKey if respective WdsEncrypType is NONE
    for i=0,3 do
        if (cfgs["Wds"..i.."Key"] and cfgs["Wds"..i.."Key"] ~= "") and
           ((not mtkwifi.token_get(cfgs["WdsEncrypType"],i+1,nil)) or
            ("NONE" == mtkwifi.token_get(cfgs["WdsEncrypType"],i+1,nil))) then
            cfgs["Wds"..i.."Key"] = ""
        end
    end

    cfgs["AccessPolicy"..vif_idx-1] = http.formvalue("__accesspolicy")
    local t = mtkwifi.parse_mac(http.formvalue("__maclist"))
    cfgs["AccessControlList"..vif_idx-1] = table.concat(t, ";")

    __security_cfg(cfgs, vif_idx)
    __update_mbss_para(cfgs, vif_idx)
    __set_wifi_wpsconf(cfgs, http.formvalue("WPSRadio"), vif_idx)

    __mtkwifi_save_cfg(cfgs, devname)

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",to_url))
    else
        luci.http.redirect(to_url)
    end
end


function getSsid(ifname)
    local fp = io.popen("hostapd_cli -i "..ifname.." status | grep ^ssid | cut -d = -f 2")
    local result = fp:read("*all")
    fp:close()
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        if line and line ~= '' then
            return line
        end
    end

    return nil
end

function getHostapdCfg(ifname)
    fp = io.popen("hostapd_cli -i"..ifname.." get_config")
    result = fp:read("*all")
    fp:close()
    lines = mtkwifi.__lines(result)
    local cfg = {}
    for i, line in pairs(lines) do
        _, _, k, v = line:find("([%w_-]+)%s*=%s*(.+)")
        if k and v then
            cfg[k] = v
        end
    end

    return cfg
end

function getCurrentWscProfile(ifname)
    local i, k, v
    local fp = io.popen("hostapd_cli -i"..ifname.." WPS_GET_STATUS")
    local result = fp:read("*all")
    fp:close()

    local profile = {}
    local lines = mtkwifi.__lines(result)

    for i, line in pairs(lines) do
        v = line:match("Last WPS result:%s*(.+)")
        if v then
            profile['WscResult'] = v
        else
            v = line:match("Peer Address:%s*(.+)")
            if v then
                profile['Peer Address'] = v
            end
        end
    end

    local wpsInfo = getHostapdCfg(ifname)
    for k,v in pairs(wpsInfo) do
        profile[k] = v
    end

    local AuthMode, EncType
    if profile["key_mgmt"] and profile["wpa"] and profile["rsn_pairwise_cipher"] then
        AuthMode, EncType = mtkwifi.host2datmode(profile["key_mgmt"], profile["wpa"], profile["rsn_pairwise_cipher"])
    else
        AuthMode = "OPEN"
        EncType = "NONE"
    end
    profile["SSID"] = profile["ssid"]
    profile["WscWPAKey"] = profile["passphrase"]
    profile["AuthMode"] = AuthMode
    profile["EncType"] = EncType

    return profile
end

function get_WPS_Info(devname, ifname)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx

    local cfgs = mtkwifi.load_cfg(devname, "", true)

    local path = '/etc/config/wireless'
    -- Create the applied settings backup file if it does not exist.
    if not mtkwifi.exists(mtkwifi.__uci_applied_settings_path()) then
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end
    local applied_cfgs = mtkwifi.load_cfg(devname, mtkwifi.__uci_applied_config(), true)

    local WPS_details = {}
    WPS_details =  getCurrentWscProfile(ifname) -- c_getCurrentWscProfile(ifname)

    if type(WPS_details) ~= "table" then
        WPS_details["DRIVER_RSP"] = "NO"
    else
        WPS_details["DRIVER_RSP"] = "YES"
        local isCfgsChanged = false  -- To indicate that the settings have been changed by External Registrar.
        local isBasicTabUpdateRequired = false

        if type(WPS_details["SSID"]) == "string" then
            if applied_cfgs["SSID"..ssid_index] ~= WPS_details["SSID"] then
                cfgs["SSID"..ssid_index] = WPS_details["SSID"]
                isCfgsChanged = true
                isBasicTabUpdateRequired = true
            end
        else
            WPS_details["SSID"] = cfgs["SSID"..ssid_index]
        end

        if type(WPS_details["AuthMode"]) == "string" then
            local auth_mode_ioctl = WPS_details["AuthMode"]:gsub("%W",""):upper()
            local auth_mode_applied = mtkwifi.token_get(applied_cfgs.AuthMode, ssid_index, "")
            if auth_mode_applied ~= auth_mode_ioctl then
                cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, ssid_index, auth_mode_ioctl)
                isCfgsChanged = true
                isBasicTabUpdateRequired = true
            end
        else
            WPS_details["AuthMode"] = mtkwifi.token_get(cfgs.AuthMode, ssid_index, "")
        end

        if type(WPS_details["EncType"]) == "string" then
            local enc_type_ioctl = WPS_details["EncType"]:upper()
            local enc_type_applied = mtkwifi.token_get(applied_cfgs.EncrypType, ssid_index, "")
            if enc_type_applied ~= enc_type_ioctl then
                cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, ssid_index, enc_type_ioctl)
                isCfgsChanged = true
                isBasicTabUpdateRequired = true
            end
        else
            WPS_details["EncType"] = mtkwifi.token_get(cfgs.EncrypType, ssid_index, "")
        end

        if type(WPS_details["WscWPAKey"]) == "string" then
            if applied_cfgs["WPAPSK"..ssid_index] ~= WPS_details["WscWPAKey"] then
                cfgs["WPAPSK"..ssid_index] = WPS_details["WscWPAKey"]
                isCfgsChanged = true
                isBasicTabUpdateRequired = true
            end
        else
            WPS_details["WscWPAKey"] = cfgs["WPAPSK"..ssid_index]
        end

        if type(WPS_details["DefKey"]) == "number" then
            local def_key_applied = tonumber(mtkwifi.token_get(applied_cfgs.DefaultKeyID, ssid_index, ""))
            if def_key_applied ~= WPS_details["DefKey"] then
                cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, ssid_index, WPS_details["DefKey"])
                isCfgsChanged = true
            end
        else
            WPS_details["DefKey"] = tonumber(mtkwifi.token_get(cfgs.DefaultKeyID, ssid_index, 0)) or ""
        end

        if WPS_details["wps_state"] == "configured" then
            WPS_details["Conf"] = "2"
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, ssid_index, WPS_details["Conf"])
        else
            WPS_details["Conf"] = "1"
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, ssid_index, WPS_details["Conf"])
        end

        WPS_details["IS_BASIC_TAB_UPDATE_REQUIRED"] = isBasicTabUpdateRequired

        if isCfgsChanged then
            -- Driver updates the *.dat file for following scenarios,
            --     1. When WPS Conf Status is not configured i.e. WscConfStatus is not set as 2,
            --        and connection with a station is established i.e. where station acts as an External Registrar.
            --     2. When below settings are changed through External Registrar irrespective of WPS Conf Status
            -- Update mtkwifi.__profile_applied_settings_path(profile) file with the
            -- new settings to avoid display of "reload to apply changes" message.
            applied_cfgs["WPAPSK"] = cfgs["WPAPSK"]
            applied_cfgs["SSID"] = cfgs["SSID"]
            applied_cfgs["SSID"..ssid_index] = cfgs["SSID"..ssid_index]
            applied_cfgs["AuthMode"] = cfgs["AuthMode"]
            applied_cfgs["EncrypType"] = cfgs["EncrypType"]
            applied_cfgs["WPAPSK"..ssid_index] = cfgs["WPAPSK"..ssid_index]
            applied_cfgs["DefaultKeyID"] = cfgs["DefaultKeyID"]
            applied_cfgs["WscConfStatus"] = cfgs["WscConfStatus"]
            mtkwifi.save_cfg(applied_cfgs, devname, mtkwifi.__uci_applied_config())
        end
    end
    http.write_json(WPS_details)
end

local function getApPin(ifname)
    local pin = ''
    local fp = io.popen("uci show wireless."..ifname..".wps_pin")
    local result = fp:read("*all")
    fp:close()

    if string.find(result, "wps_pin") then
        pin = string.gsub(result, ".*wps_pin=\'(.-)\'.*", "%1")
    else
        pin = genApPin(ifname)
    end

    return pin
end

function genApPin(ifname)
    local fp = io.popen("hostapd_cli -i"..ifname.." WPS_AP_PIN random")
    local result = fp:read("*all")
    fp:close()

    local pin = string.gsub(result, "(.-)\r?\n", "%1")

    os.execute("uci set wireless."..ifname..".wps_pin="..pin)
    os.execute("uci commit wireless."..ifname)

    return pin
end

function get_wifi_pin(ifname)
    local pin_code = getApPin(ifname)
    local pin = {}
    pin['genpincode'] = pin_code
    http.write_json(pin)
end

function set_wifi_gen_pin(ifname,devname)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx

    local cfgs = mtkwifi.load_cfg(devname, "", true)
    local pin = {}
    local pin_code = genApPin(ifname)
    pin['genpincode'] = pin_code
    cfgs["WscVendorPinCode"]=mtkwifi.token_set(cfgs["WscVendorPinCode"],ssid_index,pin_code)

    __mtkwifi_save_cfg(cfgs, devname)
    http.write_json(pin)
end

function set_wifi_wps_oob(devname, ifname)
    local SSID, mac = ""
    local  ssid_index = 0
    local devs = mtkwifi.get_all_devs()

    local cfgs = mtkwifi.load_cfg(devname, "", true)

    ssid_index = devs[devname]["vifs"][ifname].vifidx
    local rd_pipe_output = mtkwifi.read_pipe("cat /sys/class/net/"..ifname.."/address 2>/dev/null")
    mac = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or ""
    local c_band =  mtkwifi.band(string.split(cfgs.WirelessMode,";")[1])

    local dev_name = string.sub(devname, 1, 6)
    if mac ~= "" then
        SSID = dev_name.."_"..c_band.."_"..mac
    else
        SSID = dev_name.."_"..c_band.."_".."_unknown"
    end

    cfgs["SSID"..ssid_index]=SSID
    cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, ssid_index, "1")
    cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, ssid_index, "WPA2PSK")
    cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, ssid_index, "AES")
    cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, ssid_index, "2")
    cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, ssid_index, "0")
    cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, ssid_index, "0")

    math.randomseed(os.time())
    local wps_key = math.random(10000000,99999999)
    cfgs["WPAPSK"..ssid_index]=wps_key
    cfgs["WPAPSK"]=""
    cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, ssid_index, "0")

    __mtkwifi_save_cfg(cfgs, devname)
    mtkwifi.__run_in_child_env(__mtkwifi_restart, devname)
    mtkwifi.__run_in_child_env(__restart_all_daemons, devname, ifname)

    local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "wifi", "vif_cfg_view", devname, ifname)
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))

    cfgs = mtkwifi.__restart_if_wps(devname, ifname, cfgs)
end

function set_wifi_do_wps(ifname, devname, wsc_pin_code_w)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx
    local wsc_mode = 0
    local wsc_conf_mode

    local cfgs = mtkwifi.load_cfg(devname, "", true)

    if(wsc_pin_code_w == "nopin") then
        wsc_mode=2
    else
        wsc_mode=1
    end

    wsc_conf_mode = mtkwifi.token_get(cfgs["WscConfMode"], ssid_index, nil)

    if (wsc_conf_mode == 0) then
        print("{\"wps_start\":\"WPS_NOT_ENABLED\"}")
        DBG_MSG("WPS is not enabled before do PBC/PIN.\n")
        return
    end

    if ( wsc_mode == 1 and wsc_mode == 2 ) then
        http.write_json("{\"wps_start\":\"NG\"}")
        return
    end
    cfgs["WscStartIF"] = ifname

    http.write_json("{\"wps_start\":\"OK\"}")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "vif_cfg_view",devname,ifname))

    if (wsc_mode == 1) then
        __wps_ap_pin_start_all(ifname, wsc_pin_code_w)
    elseif (wsc_mode == 2) then
        __wps_ap_pbc_start_all(ifname)
    end

end

function get_wps_security(ifname, devname)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx
    local output = {}
    local cfgs = mtkwifi.load_cfg(devname)

    output["AuthMode"] = mtkwifi.token_get(cfgs.AuthMode,ssid_index)
    output["IEEE8021X"] = mtkwifi.token_get(cfgs.IEEE8021X,ssid_index)

    http.write_json(output)
end

function getCurrentStatus(num)
    local v = ""
    if num == 0 then
        v = 'SEND_M1'
    elseif num == 1 then
        v = 'RECV_M2'
    elseif num == 2 then
        v = 'SEND_M3'
    elseif num == 3 then
        v = 'RECV_M4'
    elseif num ==4 then
        v = 'SEND_M5'
    elseif num == 5 then
        v = 'RECV_M6'
    elseif num == 6 then
        v = 'SEND_M7'
    elseif num == 7 then
        v = 'RECV_M8'
    elseif num == 8 then
        v = 'RECEIVED_M2D'
    elseif num == 9 then
        v = 'WPS_MSG_DONE'
    elseif num == 10 then
        v = 'RECV_ACK'
    elseif num == 11 then
        v = 'WPS_FINISHED'
    elseif num == 12 then
        v = 'SEND_WSC_NACK'
    elseif num == 13 then
        v = 'MULTIPLE_PBC_DETECTED'
    end
    return v
end

function getCurrentApcliWscProfile(ifname)
    local i, k, v, current_status
    local fp = io.popen("wpa_cli -i"..ifname.." status")
    local result = fp:read("*all")
    fp:close()

    local profile = {}
    profile['apcli_get_wps_status'] = 'OK'
    local lines = mtkwifi.__lines(result)

    for i, line in pairs(lines) do
        v = line:match("wpa_state=%s*(.+)")
        if v then
            profile['wps_result'] = v
        end
        current_status = line:match("current WPS_STATUS=%s*(.+)")
        if current_status then
            profile["current_status"] = getCurrentStatus(tonumber(current_status))
        end
    end

    return profile
end

function apcli_get_wps_status(ifname, devname)
    local output = {}
    local  ssid_index = 0
    local devs = mtkwifi.get_all_devs()

    -- apcli interface has a different structure as compared to other vifs
    ssid_index = devs[devname][ifname].vifidx
   -- output = c_apcli_get_wps_status(ifname)
    output = getCurrentApcliWscProfile(ifname)

    http.write_json(output);
end

function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

function unencode_ssid(raw_ssid)
    local c
    local output = ""
    local convertNext = 0
    for c in raw_ssid:gmatch"." do
        if(convertNext == 0) then
            if(c == '+') then
                output = output..' '
            elseif(c == '%') then
                convertNext = 1
            else
                output = output..c
            end
        else
            output = output..string.tohex(c)
            convertNext = 0
        end
    end
    return output
end

function decode_ssid(raw_ssid)
    local output = raw_ssid
    output = output:gsub("&amp;", "&")
    output = output:gsub("&lt;", "<")
    output = output:gsub("&gt;", ">")
    output = output:gsub("&#34;", "\"")
    output = output:gsub("&#39;", "'")
    output = output:gsub("&nbsp;", " ")
    for codenum in raw_ssid:gmatch("&#(%d+);") do
        output = output:gsub("&#"..codenum..";", string.char(tonumber(codenum)))
    end
    return output
end

function apcli_do_enr_pin_wps(ifname, devname, pin_code)
    http.write_json("{\"apcli_do_enr_pin_wps\":\"OK\"}")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "apcli_cfg_view",devname,ifname))
    os.execute("ifconfig "..ifname.." up")
    debug_write("ifconfig "..ifname.." up")
    os.execute("brctl addif br0 "..ifname)
    debug_write("brctl addif br0 "..ifname)
    os.execute("brctl addif br-lan "..ifname)
    debug_write("brctl addif br-lan "..ifname)
    os.execute("wpa_cli -i"..ifname.." wps_pin any "..pin_code)
    debug_write("wpa_cli -i"..ifname.." wps_pin any "..pin_code)
end

function apcli_do_enr_pbc_wps(ifname, devname)
    http.write_json("{\"apcli_do_enr_pbc_wps\":\"OK\"}")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "apcli_cfg_view",devname,ifname))
    os.execute("wpa_cli -i "..ifname.." wps_pbc")
end

function apcli_cancel_wps(ifname)
    http.write_json("{\"apcli_cancel_wps\":\"OK\"}")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "apcli_cfg_view",devname,ifname))
    os.execute("wpa_cli -i"..ifname.." wps_cancel")
    os.execute("miniupnpd.sh init")
end

function apcli_wps_gen_pincode(ifname)
    local fp = io.popen("wpa_cli -i"..ifname.." wps_pin get")
    local result = fp:read("*all")
    fp:close()
    local pin = string.gsub(result, "(.-)\r?\n", "%1")

    os.execute("uci set wireless."..ifname..".wps_pin="..pin)
    os.execute("uci commit wireless."..ifname)

    local ret_value = {}
    ret_value["apcli_wps_gen_pincode"] = "OK"
    http.write_json(ret_value)
end

function apcli_wps_get_pincode(ifname)
    local pin = ''
    local fp = io.popen("uci show wireless."..ifname..".wps_pin")
    local result = fp:read("*all")
    fp:close()

    pin = string.gsub(result, ".*wps_pin=\'(.-)\'.*", "%1")
    if not pin or pin == nil or pin == '' then
        fp = io.popen("wpa_cli -i"..ifname.." wps_pin get")
        result = fp:read("*all")
        fp:close()
        pin = string.gsub(result, "(.-)\r?\n", "%1")
        os.execute("uci set wireless."..ifname..".wps_pin="..pin)
        os.execute("uci commit wireless."..ifname)
    end
    local output = {}
    output['getpincode'] = pin
    http.write_json(output)
end

function get_apcli_conn_info(ifname)
    local rsp = {}
    if not ifname then
        rsp["conn_state"]="Disconnected"
    else
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..ifname.."/flags 2>/dev/null")) or 0
        rsp["infc_state"] = flags%2 == 1 and "up" or "down"
        local iwapcli = mtkwifi.read_pipe("iwconfig "..ifname.." | grep ESSID 2>/dev/null")
        local ssid = string.match(iwapcli, "ESSID:\"(.*)\"")
        iwapcli = mtkwifi.read_pipe("iwconfig "..ifname.." | grep 'Access Point' 2>/dev/null")
        local bssid = string.match(iwapcli, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x")
        if not ssid or ssid == "" then
            rsp["conn_state"]= "Disconnected"
        else
            rsp["conn_state"] = "Connected"
            rsp["ssid"] = ssid
            rsp["bssid"] = bssid or "N/A"
        end
    end
    http.write_json(rsp)
end

function sta_info(ifname)
    local output = {}
    local stalist = mtkwifi.get_sta_list(ifname)
    local count = 0
    for _ in pairs(stalist) do count = count + 1 end

    for i=1, count do
        table.insert(output, stalist[i])
    end
    http.write_json(output)
end

function apcli_scan(ifname)
    local aplist = mtkwifi.scan_ap(ifname)
    --local convert="";
    --for i=1, #aplist do
        --convert = c_convert_string_display(aplist[i]["ssid"])
        --aplist[i]["original_ssid"] = aplist[i]["ssid"]
        --aplist[i]["ssid"] = convert["output"]
    --end
    http.write_json(aplist)
end

function get_station_list()
    http.write("get_station_list")
end

function reset_wifi(devname)
    if devname then
        os.execute("cp -f /rom/etc/wireless/"..devname.."/ /etc/wireless/")
    else
        os.execute("cp -rf /rom/etc/wireless /etc/")
    end
    return luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
end

function reload_wifi(devname)
    mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
    local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "wifi")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
end

function reload_mld()
    mtkwifi.__run_in_child_env(__mtkwifi_restart)
    local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "mld")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
end

function get_raw_profile()
    local sid = http.formvalue("sid")
    http.write_json("get_raw_profile")
end

function get_country_region_list()
    local mode = http.formvalue("mode")
    local cr_list;

    if mtkwifi.band(mode) == "5G" then
        cr_list = mtkwifi.CountryRegionList_5G_All
    elseif mtkwifi.band(mode) == "6G" then
        cr_list = mtkwifi.CountryRegionList_6G_All
    else
        cr_list = mtkwifi.CountryRegionList_2G_All
    end

    http.write_json(cr_list)
end

function remove_ch_by_region(ch_list, region)
    for i = #ch_list,2,-1 do
        if not ch_list[i].region[region] then
            table.remove(ch_list, i)
        end
    end
end

function get_channel_list()
    local mode = http.formvalue("mode")
    local region = tonumber(http.formvalue("country_region")) or 1
    local ch_list

    if mtkwifi.band(mode) == "5G" then
        ch_list = mtkwifi.ChannelList_5G_All
    elseif mtkwifi.band(mode) == "6G" then
        ch_list = mtkwifi.ChannelList_6G_All
    else
        ch_list = mtkwifi.ChannelList_2G_All
    end

    remove_ch_by_region(ch_list, region)
    http.write_json(ch_list)
end

function get_channel_list_not_support_region()
    local main_ifname = http.formvalue("main_ifname")
    local country_code = http.formvalue("country_code")
    if country_code == "NONE" then
        country_code = "00"
    end
    local phy = mtkwifi.get_phy_by_main_ifname(main_ifname)
    os.execute("mwctl phy "..phy.." set country code="..country_code..";")
    channel_list = mtkwifi.get_channel_list_from_cmd(phy)
    http.write_json(channel_list)
end

function get_wirelessmode()
    local band = http.formvalue("band")
    if band == "5G-High" or band == "5G-Low" then
        band = "5G"
    end
    local htmode = http.formvalue("htmode")
    local cfg = (mtkwifi.htmode2mode({}, htmode, band))
    http.write_json(cfg)
end

function get_HT_ext_channel_list()
    local mode = http.formvalue("mode")
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region")) or 1
    local ext_ch_list = {}

    if mtkwifi.band(mode) == "6G" then -- 6G Channel
        local i = 1
        local ch_list = mtkwifi.ChannelList_6G_All
        local below, above
        if 1 <= ch_cur and ch_cur <= 61 then
            below = 31
        elseif 65 <= ch_cur and  ch_cur <= 125 then
            below = 95
        elseif 129 <= ch_cur and  ch_cur <= 189 then
            below = 159
        end

        if 33 <= ch_cur and  ch_cur <= 93 then
            above = 63
        elseif 97 <= ch_cur and  ch_cur <= 157 then
            above = 127
        elseif 161 <= ch_cur and  ch_cur <= 221 then
            above = 191
        end

        if below then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 0
            ext_ch_list[i].text = "Channel "..below
            i = i + 1
        end

        if above then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 1
            ext_ch_list[i].text = "Channel "..above
            i = i + 1
        end

    elseif mtkwifi.band(mode) == "2.4G" then -- 2.4G Channel
        local ch_list = mtkwifi.ChannelList_2G_All
        local below_ch = ch_cur - 4
        local above_ch = ch_cur + 4
        local i = 1

        if below_ch > 0 and ch_list[below_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 0
            ext_ch_list[i].text = ch_list[below_ch + 1].text
            i = i + 1
        end

        if above_ch <= 14 and ch_list[above_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 1
            ext_ch_list[i].text = ch_list[above_ch + 1].text
        end
    else  -- 5G Channel
        local ch_list = mtkwifi.ChannelList_5G_All
        local ext_ch_idx = -1
        local len = 0

        for k, v in ipairs(ch_list) do
            len = len + 1
            if v.channel == ch_cur then
                ext_ch_idx = (k % 2 == 0) and k + 1 or k - 1
            end
        end

        if ext_ch_idx > 0 and ext_ch_idx < len and ch_list[ext_ch_idx].region[region] then
            ext_ch_list[1] = {}
            ext_ch_list[1].val = ext_ch_idx % 2
            ext_ch_list[1].text = ch_list[ext_ch_idx].text
        end
    end

    http.write_json(ext_ch_list)
end

function get_5G_2nd_80Mhz_channel_list()
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region"))
    local ch_list = mtkwifi.ChannelList_5G_2nd_80MHZ_ALL
    local ch_list_5g = mtkwifi.ChannelList_5G_All
    local i, j, test_ch, test_idx
    local bw80_1st_idx = -1

    -- remove adjacent freqencies starting from list tail.
    for i = #ch_list,1,-1 do
        for j = 0,3 do
            if ch_list[i].channel == -1 then
                break
            end

            test_ch = ch_list[i].channel + j * 4
            test_idx = ch_list[i].chidx + j

            if test_ch == ch_cur then
            if i + 1 <= #ch_list and ch_list[i + 1] then
                table.remove(ch_list, i + 1)
            end
            table.remove(ch_list, i)
                bw80_1st_idx = i
                break
            end

            if i == (bw80_1st_idx - 1) or (not ch_list_5g[test_idx].region[region]) then
                table.remove(ch_list, i)
            break
        end
    end
    end

    -- remove unused channel.
    for i = #ch_list,1,-1 do
        if ch_list[i].channel == -1 then
            table.remove(ch_list, i)
        end
    end
    http.write_json(ch_list)
end

function webcmd()
    local cmd = http.formvalue("cmd")
    if cmd then
        local result = mtkwifi.read_pipe(tostring(cmd).." 2>&1")
        result = result:gsub("<", "&lt;")
        http.write(tostring(result))
    else
        http.write_json(http.formvalue())
    end
end

function net_cfg()
    http.write_json(http.formvalue())
end

function mld_sercurity(mldgroup, auth, mode)
    local pmfmfpc, pmfmfpr, encrypttype, pmf_sha256
    if mode == "ap" then
        pmfmfpc = http.formvalue("__pmfmfpc") or "0"
        pmfmfpr = http.formvalue("__pmfmfpr") or "0"
        encrypttype = http.formvalue("encrypttype")
        pmf_sha256 = http.formvalue("pmf_sha256")
    else
        pmfmfpc = http.formvalue("sta_pmfmpc") or "0"
        pmfmfpr = http.formvalue("sta_pmfmfpr") or "0"
        encrypttype = http.formvalue("sta_encry")
        pmf_sha256 = http.formvalue("sta_sha256")
    end

    local ieee80211w = "0"
    if pmfmfpc == '1' and pmfmfpr == '1' then
        ieee80211w = '2'
    elseif pmfmfpc == '1' then
        ieee80211w = '1'
    else
        ieee80211w = '0'
    end
    if auth == "Disable" then
        mldgroup.auth = "OPEN"
        mldgroup.encr = "NONE"
    elseif auth == "Enhanced Open" then
        mldgroup.auth = "OWE"
        mldgroup.encr = "AES"
        mldgroup.ieee80211w = "2"
        mldgroup.pmf_sha256 = "0"

    elseif auth == "WPAPSK"  then
        mldgroup.encr = encrypttype or "AES"
        mldgroup.rekey_method = "TIME"

    elseif auth == "WPAPSKWPA2PSK" then
        mldgroup.encr = encrypttype or "AES"
        mldgroup.rekey_method = "TIME"

    elseif auth == "WPA2PSK" then
        mldgroup.encr = encrypttype or "AES"
        mldgroup.rekey_method = "TIME"
        mldgroup.ieee80211w = ieee80211w
        mldgroup.pmf_sha256 = pmf_sha256 or "0"

    elseif auth == "WPA3PSK" then
        mldgroup.encr = encrypttype
        mldgroup.rekey_method = "TIME"
        mldgroup.ieee80211w = '2'
        mldgroup.pmf_sha256 = "0"

    elseif auth == "WPA2PSKWPA3PSK" then
        mldgroup.encr = encrypttype
        mldgroup.rekey_method = "TIME"
        mldgroup.ieee80211w = '1'
        mldgroup.pmf_sha256 = "0"

    elseif auth == "WPA2" then
        mldgroup.encr = encrypttype or "AES"
        mldgroup.rekey_method = "TIME"
        -- for DOT11W_PMF_SUPPORT
        mldgroup.ieee80211w = ieee80211w
        mldgroup.pmf_sha256 = pmf_sha256 or "0"

    elseif auth == "WPA3" then
        mldgroup.encr = "AES"
        mldgroup.rekey_method = "TIME"
        -- for DOT11W_PMF_SUPPORT
        mldgroup.ieee80211w = '2'
        mldgroup.pmf_sha256 = "0"

    elseif auth == "WPA3-192-bit" then
        mldgroup.auth = "WPA3-192"
        mldgroup.encr = "GCMP256"
        mldgroup.rekey_method = "TIME"
        -- for DOT11W_PMF_SUPPORT
        mldgroup.ieee80211w = '2'
        mldgroup.pmf_sha256 = "0"

    elseif auth == "WPA1WPA2" then
        mldgroup.encr = encrypttype or "AES"
        mldgroup.rekey_method = "TIME"
        -- for DOT11W_PMF_SUPPORT
        mldgroup.ieee80211w = ieee80211w
        mldgroup.pmf_sha256 = pmf_sha256 or "0"

    elseif auth == "IEEE8021X" then
        mldgroup.auth = "OPEN"
        mldgroup.encr = http.formvalue("__8021x_wep") and "WEP" or "NONE"
        mldgroup.IEEE8021X = "1"
    end
end

function enable_all_apcli()
    local devs = mtkwifi.get_all_devs()
    local apcli_list = {}
    for _, dev in ipairs(devs) do
        apcli_list[mtkwifi.band(dev.WirelessMode)] = dev.apcli
    end

    for _, apcli in pairs(apcli_list) do
        os.execute("ifconfig "..apcli.vifname.." up")
        os.execute("uci set wireless."..apcli.vifname..".disabled=0")
    end
    os.execute("uci commit wireless")
end

function mld_cfg()
    local mldgroup = mtkwifi.load_cfg("","",true)
    local mld_len, mld_mode
    if not mldgroup.mld then
        mldgroup.mld = {}
        mld_len = 0
    else
        mld_len = #mldgroup.mld
    end
    local idx = {}
    local mld_id = 0
    local mld_name = ""
    local mld_group = {}

    for _,mld in pairs(mldgroup.mld) do
        table.insert(idx, string.match(mld["name"], "%d+"))
    end

    if http.formvalue("mode") == "ap" then mld_mode = "apmld" else mld_mode = "stamld" end

    if http.formvalue("apmld") == nil or http.formvalue("apmld") == "" then
        for i=1, 63 do
            if mtkwifi.is_include(i, idx) then
                mld_id = i
                break;
            end
        end
        mld_group.name = mld_mode..mld_id
    else
        for _,mld in pairs(mldgroup.mld) do
            if http.formvalue("apmld") == mld["name"] then
                mld_group = mld
            end
        end
    end

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "dev_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "dev_cfg, special: "..k.."="..v)
        else
            mld_group[k] = v or ""
        end
    end

    local auth
    if mld_group.mode == "ap" then
        auth = http.formvalue("authmode") or ""
        mld_group.key = http.formvalue("key")
        mld_group.main_iface = ""
    else
        mld_group.key = http.formvalue("ApCliWPAPSK")
        auth = http.formvalue("sta_authmode") or ""
    end
    mld_group.auth = auth
    local iface_1 = http.formvalue("1_list") and http.formvalue("1_list").." " or ""
    local iface_2 = http.formvalue("2_list") and http.formvalue("2_list").." " or ""
    local iface_3 = http.formvalue("3_list") and http.formvalue("3_list").." " or ""
    mld_group.iface = mtkwifi.__trim(iface_1..iface_2..iface_3)
    mld_sercurity(mld_group, auth, mld_group.mode)

    if http.formvalue("apmld") == nil or http.formvalue("apmld") == "" then
        mldgroup.mld[mld_len+1] = mld_group
    else
        for _,mld in pairs(mldgroup.mld) do
            if http.formvalue("apmld") == mld["name"] then
                mld = mld_group
            end
        end
    end

    __mtkwifi_save_cfg(mldgroup, mld_name)

    if http.formvalue("__apply") then
        if mld_group.mode == "sta" and mld_group.disabled == "0" then
            enable_all_apcli()
        end
        mtkwifi.__run_in_child_env(__mtkwifi_restart)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "mld")
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "mld"))
    end
end

function mld_del(mld_name)
    local path = "/etc/config/wireless"
    if not mtkwifi.exists(mtkwifi.__uci_applied_settings_path()) then
        os.execute("cp -f "..path.." "..mtkwifi.__uci_applied_settings_path())
    end

    os.execute("uci delete wireless."..mld_name)
    os.execute("uci commit")

    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "mld"))
end

function mld_edit(mld_name)
    local mldgroup = mtkwifi.load_cfg("","",true)
    local mldObj = {}
    for _,mld in ipairs(mldgroup.mld) do
        if mld.name == mld_name then
            mldObj = mld
        end
    end
    http.write_json(mldObj)
end

function iface_disable(vifs)
    local iface_str = get_mld_iface_list()
    for _, vif in ipairs(vifs) do
        if string.find(iface_str, vif.vifname) then
            vif.disable_iface = true
        end
    end
    return vifs
end

function get_iface_list(mode)
    local devs = mtkwifi.get_all_devs()
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX
    local list = {}
    list.htmode={}
    for _, dev in ipairs(devs) do
        local devname = dev.devname
        local main_ifname = l1dat and l1dat[dridx][devname].main_ifname
        local phy = mtkwifi.iface_type(main_ifname)
        local res = mtkwifi.get_band(phy)
        table.insert(list.htmode,dev.htmode)

        if mode == "ap" then
            list[res] = iface_disable(dev.vifs)
        else
            list[res] = dev.apcli
        end
    end
    http.write_json(list)
end

function get_mld_iface_list()
    local cfgs = mtkwifi.load_cfg("","",true)
    local iface_str = "", result
    if cfgs.mld then
        for _, mld in ipairs(cfgs.mld) do
            local fp = io.popen("uci show wireless."..mld["name"].." | grep iface | cut -d = -f 2")
            result = fp:read("*all")
            fp:close()

            local lines = mtkwifi.__lines(result)
            for i, line in pairs(lines) do
                if line and line ~= '' then
                    rs = mtkwifi.__trim(line)
                    iface_str = iface_str..rs
                end
            end
        end
    end
    return iface_str
end

function apcli_cfg(dev, vif)
    local devname = dev
    debug_write(devname)

    local cfgs = mtkwifi.load_cfg(devname, "", true)

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "apcli_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            cfgs[k] = v or ""
        end
    end

    __mtkwifi_save_cfg(cfgs, devname, false)

    if http.formvalue("__apply") then
        if cfgs.ApCliEnable == "1" then
            os.execute("ifconfig "..vif.." up")
        end
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname, true)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "wifi", "apcli_cfg_view", dev, vif)
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "apcli_cfg_view", dev, vif))
    end
end

function get_key_mgmt(auth, encr)
    local wpa_key_mgmt = ''
    local rsn_pairwise = ''

    if auth == "OPEN" then
        wpa_key_mgmt = 'NONE'
    elseif auth == "SHARED" then
        wpa_key_mgmt = 'NONE'
    elseif auth == "WPAPSK" and encr == "AES" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'CCMP'
     elseif auth == "WPAPSK" and encr == "TKIP" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'TKIP'
     elseif auth == "WPAPSK" and encr == "TKIPAES" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'TKIP CCMP'
    elseif auth == "WPA2PSK" and encr == "AES" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'CCMP'
     elseif auth == "WPA2PSK" and encr == "TKIP" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'TKIP'
     elseif auth == "WPA2PSK" and encr == "TKIPAES" then
        wpa_key_mgmt = 'WPA-PSK'
        rsn_pairwise = 'TKIP CCMP'
    elseif auth == "WPA3PSK" and encr == "AES" then
        wpa_key_mgmt = 'SAE'
        rsn_pairwise = 'CCMP'
    end

    return wpa_key_mgmt, rsn_pairwise
end

function apcli_connect(dev, vif)
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
    os.execute("wpa_cli -i "..vif.." reconnect")
end

function apcli_disconnect(dev, vif)
    -- dev_vif can be
    --  1. mt7620.apcli0         # simple case
    --  2. mt7615e.1.apclix0     # multi-card
    --  3. mt7615e.1.2G.apclix0  # multi-card & multi-profile
    local devname,vifname = dev, vif
    debug_write("devname=", dev, "vifname", vif)
    debug_write(devname)
    debug_write(vifname)
    local cfgs = mtkwifi.load_cfg(devname, "", true)
    cfgs.ApCliEnable = "1"
    __mtkwifi_save_cfg(cfgs, devname, true)
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi"))
    os.execute("wpa_cli -i "..vifname.." disconnect")
end

-- Mediatek Adaptive Network
function man_cfg()
    local mtkwifi = require("mtkwifi")

    for k,v in pairs(http.formvalue()) do
        debug_write(k.."="..v)
    end


    for _, devname in pairs(ucicfg["wifi-device"]) do
        local dev = string.gsub(devname[".name"], "%_", ".")
        local cfgs = mtkwifi.load_cfg(dev, "", true)

        if cfgs.ApCliEzEnable then

            for k,v in pairs(http.formvalue()) do
                if type(v) ~= type("") and type(v) ~= type(0) then
                    nixio.syslog("err", "man_cfg, invalid value type for "..k..","..type(v))
                elseif string.byte(k) ~= string.byte("_") then
                    cfgs[k] = v or ""
                end
            end

            debug_write(tostring(http.formvalue("__"..dev.."_ezsetup")))
            cfgs.ApCliEzEnable = http.formvalue("__"..dev.."_ezsetup") or "0"

            -- Yes this is bad. LSDK insists on this.
            if cfgs.ApCliEzEnable == "1" then
                cfgs.ApCliEnable = "1"
                cfgs.ApCliMWDS = "1"
                cfgs.ApCliAuthMode = "WPS2PSK"
                cfgs.ApCliEncrypType = AES
                cfgs.ApCliWPAPSK = "12345678"
                cfgs.AuthMode = "WPA2PSK"
                cfgs.EncrypType = "AES"
                cfgs.RekeyMethod = "TIME"
                cfgs.WPAPSK1 = ""
                cfgs.RegroupSupport = "1;1"
            end

            if http.formvalue("__group_id_mode") == "0" then
                cfgs.EzGroupID = cfgs.ApCliEzGroupID
                cfgs.EzGenGroupID = ""
                cfgs.ApCliEzGenGroupID = ""
            else
                cfgs.EzGroupID = ""
                cfgs.ApCliEzGroupID = ""
                cfgs.EzGenGroupID = cfgs.ApCliEzGenGroupID
            end

            cfgs.EzEnable = cfgs.ApCliEzEnable
            cfgs.ApMWDS = cfgs.ApCliMWDS
            cfgs.EzConfStatus = cfgs.ApCliEzConfStatus
            cfgs.EzOpenGroupID = cfgs.ApCliEzOpenGroupID
        end
        __mtkwifi_save_cfg(cfgs, dev, false)
    end

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "man")
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "man"))
    end
end

function apply_power_boost_settings()
    local devname = http.formvalue("__devname")
    local ret_status = {}
    local devs = mtkwifi.get_all_devs()
    local dev = {}
    for _,v in ipairs(devs) do
        if v.devname == devname then
            dev = v
            break
        end
    end
    if next(dev) == nil then
        ret_status["status"]= "Device "..(devname or "").." not found!"
    elseif not dev.isPowerBoostSupported then
        ret_status["status"]= "Power Boost feature is not supported by "..(devname or "").." Device!"
    else
        local cfgs = mtkwifi.load_cfg(devname, "", true)
        if type(cfgs) ~= "table" or next(cfgs) == nil then
            ret_status["status"]= "Profile settings file not found!"
        else
            for k,v in pairs(http.formvalue()) do
                if type(v) ~= type("") and type(v) ~= type(0) then
                    debug_write("ERROR: [apply_power_boost_settings] String expected; Got"..type(v).."for"..k.."key")
                    ret_status["status"]= "Power Boost settings are of incorrect type!"
                    break
                elseif string.byte(k) ~= string.byte("_") then
                    cfgs[k] = v or ""
                end
            end
            if next(ret_status) == nil then
                if type(dev.vifs) ~= "table" or next(dev.vifs) == nil or not cfgs.BssidNum or cfgs.BssidNum == "0" then
                    ret_status["status"]= "No Wireless Interfaces has been added yet!"
                elseif cfgs.PowerUpenable ~= "1" then
                    ret_status["status"]= "Power Boost feature is not enabled!"
                else
                    local up_vif_name_list = {}
                    for idx,vif in ipairs(dev.vifs) do
                        if vif.state == "up" and vif.vifname ~= nil and vif.vifname ~= "" and type(vif.vifname) == "string" then
                            up_vif_name_list[idx] = vif.vifname
                        end
                    end
                    if next(up_vif_name_list) == nil then
                        ret_status["status"]= "No Wireless Interfaces is up!"
                    else
                        for _,vifname in ipairs(up_vif_name_list) do
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=0:"..cfgs.PowerUpCckOfdm)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=1:"..cfgs.PowerUpHT20)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=2:"..cfgs.PowerUpHT40)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=3:"..cfgs.PowerUpVHT20)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=4:"..cfgs.PowerUpVHT40)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=5:"..cfgs.PowerUpVHT80)
                            os.execute("mwctl "..vifname.." set TxPowerBoostCtrl=6:"..cfgs.PowerUpVHT160)
                            os.execute("sleep 1") -- Wait for 1 second to let driver process the above data
                        end
                        __mtkwifi_save_cfg(cfgs, dev, true)
                        ret_status["status"]= "SUCCESS"
                    end
                end
            end
        end
    end
    http.write_json(ret_status)
end

function get_bssid_num(devName)
    local ret_status = {}
    for _, devN in pairs(ucicfg["wifi-device"]) do
        local dev = string.gsub(devN[".name"], "%_", ".")
        if devName == dev then
            local cfgs = mtkwifi.load_cfg(devName)
            if type(cfgs) ~= "table" or next(cfgs) == nil then
                ret_status["status"]= "Wireless settings file not found!"
            else
                ret_status["status"] = "SUCCESS"
                ret_status["bssidNum"] = cfgs.BssidNum
            end
            break
        end
    end
    if next(ret_status) == nil then
        ret_status["status"]= "Device "..(devName or "").." not found!"
    end
    http.write_json(ret_status)
end

local exec_reset_to_defaults_cmd = function (devname)
    if devname then
        os.execute("wifi reset "..devname)
    else
        os.execute("wifi reset")
    end
end

function reset_to_defaults(devname)
    mtkwifi.__run_in_child_env(exec_reset_to_defaults_cmd, devname)
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",mtkwifi.get_referer_url()))
end

function get_qosmgmt_rule_list(qostype)
    local ret_status = {}
    local fp = io.popen("wappctrl ra0 get_qos_config "..qostype)
    result = fp:read("*all")
    fp:close()

    rule_str = ""
    local lines = mtkwifi.__lines(result)
    for i, line in pairs(lines) do
        if line and line ~= "" and string.find(line, qostype) then
            rule_str = rule_str..line.."\n"
            ret_status["status"] = "SUCCESS"
        end
    end
    ret_status["rules"] = rule_str

    http.write_json(ret_status)
end

function qosmgmt_config_done()
    os.execute("wappctrl ra0 save_qos_config")
end

function qosmgmt_config_change(rule_str)
    os.execute("wappctrl ra0 set_qos_config "..rule_str)
end

local exec_reset_to_default_easymesh_cmd = function ()
    -- OpenWRT
    if mtkwifi.exists("/usr/bin/EasyMesh_openwrt.sh") then
        os.execute("/usr/bin/EasyMesh_openwrt.sh default")
    elseif mtkwifi.exists("/usr/bin/EasyMesh_7622.sh") then
        os.execute("/usr/bin/EasyMesh_7622.sh default")
    elseif mtkwifi.exists("/usr/bin/EasyMesh_7629.sh") then
        os.execute("/usr/bin/EasyMesh_7629.sh default")
    end
    -- LSDK
    if mtkwifi.exists("/sbin/EasyMesh.sh") then
        os.execute("EasyMesh.sh default")
    end
end

function reset_to_default_easymesh()
    mtkwifi.__run_in_child_env(exec_reset_to_default_easymesh_cmd)
    local fd = io.open("cat /etc/config/mapd | grep dpp_cfg")
    if fd then
        os.execute("uci set mapd.dpp_cfg.allowed_role=1")
        os.execute("uci commit")
        mtkwifi.save_mesh_profile()
    end
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",mtkwifi.get_referer_url()))
end

function save_easymesh_driver_profile(easymesh_cfgs)
    local detected_5g = false
    -- Following EasyMesh settings must be written to all DAT files of Driver,
    --    1. MapEnable
    --    2. MAP_Turnkey
    local ucicfg = mtkwifi.uci_load_wireless("wireless")
    for _, dev in pairs(ucicfg["wifi-device"]) do
        local devname = string.gsub(dev[".name"], "%_", ".")
        local driver_cfgs = mtkwifi.load_cfg(devname, "", true)
        driver_cfgs['MapMode'] = easymesh_cfgs['MapMode']
        if http.formvalue("TriBand") == "1" then
            if detected_5g == false and mtkwifi.band(string.split(driver_cfgs.WirelessMode,";")[1]) == "5G" then
                driver_cfgs['ChannelGrp'] = "0:0:1:1"
                detected_5g = true
            elseif detected_5g == true and mtkwifi.band(string.split(driver_cfgs.WirelessMode,";")[1]) == "5G" then
                driver_cfgs['ChannelGrp'] = "1:1:0:0"
            end
        elseif http.formvalue("TriBand") == "2" then
            if detected_5g == false and mtkwifi.band(string.split(driver_cfgs.WirelessMode,";")[1]) == "5G" then
                driver_cfgs['ChannelGrp'] = "1:1:0:0"
                detected_5g = true
            elseif detected_5g == true and mtkwifi.band(string.split(driver_cfgs.WirelessMode,";")[1]) == "5G" then
                driver_cfgs['ChannelGrp'] = "0:0:1:1"
            end
        end
        if driver_cfgs['MapMode'] == "1" then
            driver_cfgs['SREnable'] = "0"
            driver_cfgs['SRMode'] = "0"
        end

        if driver_cfgs['MapMode'] == "2" then
            driver_cfgs['rrm_beacon_report'] = "1"
            driver_cfgs['bss_transtion'] = "1"
            os.execute("datconf -f /etc/map/mapd_default.cfg set CentralizedSteering 0")
        else
            driver_cfgs['rrm_beacon_report'] = ""
            driver_cfgs['bss_transtion'] = ""
            os.execute("datconf -f /etc/map/mapd_default.cfg set CentralizedSteering 1")
        end

        if easymesh_cfgs['MeshSREnable'] == "1" then
            driver_cfgs['SREnable'] = "1"
            driver_cfgs['SRMode'] = "2"
            driver_cfgs['BSSColorValue'] = "255"
        elseif easymesh_cfgs['MeshSREnable'] == "0" then
            driver_cfgs['SREnable'] = "0"
            driver_cfgs['SRMode'] = "0"
            driver_cfgs['BSSColorValue'] = "255"
        end
        __mtkwifi_save_cfg(driver_cfgs, devname)
    end
end

function map_cfg()
    local easymesh_cfgs = {}

    local easymesh_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__write_easymesh_profile_path())
    os.execute("cp -f "..mtkwifi.__write_easymesh_profile_path().." "..easymesh_applied_path)

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            debug_write("map_cfg: Invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            debug_write("map_cfg: Copying key:"..k..","..type(v))
            easymesh_cfgs[k] = v or ""
        end
    end

    local bands = mtkwifi.detect_triband()
    if bands ~= 3 then
        easymesh_cfgs['BhPriority5GH'] = easymesh_cfgs['BhPriority5GL']
    end

    if tostring(easymesh_cfgs['MapMode']) == "0" then
        os.execute("uci delete dhcp.lan.ignore")
        os.execute("uci commit")
        os.execute("/etc/init.d/dnsmasq reload")
    end

    save_easymesh_driver_profile(easymesh_cfgs)
    mtkwifi.save_write_easymesh_profile(easymesh_cfgs)

    if http.formvalue("__apply") then

    local fd = mtkwifi.read_pipe("cat /etc/config/mapd | grep dpp_cfg")
        if fd ~= "" then
            if http.formvalue("DeviceRole")=="1" then
                 os.execute("uci set mapd.dpp_cfg.allowed_role=2")
            elseif  http.formvalue("DeviceRole")== "2" then
                 os.execute("uci set mapd.dpp_cfg.allowed_role=1")
            elseif  http.formvalue("DeviceRole")== "0" then
                 os.execute("uci set mapd.dpp_cfg.allowed_role=0")
            end
            os.execute("uci commit")
            mtkwifi.save_mesh_profile()
        end

        if mtkwifi.exists("/usr/bin/map_restart.sh") then
            mtkwifi.__run_in_child_env(exec_map_restart)
        else
            mtkwifi.__run_in_child_env(__mtkwifi_reload)
        end

        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "mtk", "multi_ap")
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "wifi", "loading",url_to_visit_after_reload))
        if http.formvalue("__ChangeDeviceRole")=="changed" then
            os.execute("wappctrl ra0 dpp dpp_reset_dpp_config_file")
        end
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap"))
    end
end

function exec_map_restart()
    if mtkwifi.exists("/usr/bin/map_restart.sh") then
        os.execute("/usr/bin/map_restart.sh")
    end
end

function get_device_role()
    local devRole = c_get_device_role()
    -- Set ApCliEnable as "1" for Device with on-boarded ApCli interface to let
    -- UI display connection information of ApCli interface on Wireless Overview web-page.
    if tonumber(devRole.mapDevRole) == 2 then
        local r = mtkwifi.get_easymesh_on_boarded_iface_info()
        if r['status'] == "SUCCESS" then
            for vifname in string.gmatch(r['staBhInfStr'],'%a+%d') do
                local ApCliEnable = mtkwifi.read_pipe("uci get wireless."..vifname..".disabled") or ""
                if ApCliEnable ~= "0" or ApCliEnable == nil then
                    os.execute("uci set wireless."..vifname..".disabled=0")
                    os.execute("uci commit")
                end
            end
        end
    end
    http.write_json(devRole)
end

function trigger_uplink_ap_selection()
    local r = c_trigger_uplink_ap_selection()
    http.write_json(r)
end

function trigger_mandate_steering_on_agent(sta_mac, target_bssid)
    sta_mac = sta_mac:sub(1,17)
    target_bssid = target_bssid:sub(1,17)
    local r = c_trigger_mandate_steering_on_agent(sta_mac, target_bssid)
    http.write_json(r)
end

function trigger_back_haul_steering_on_agent(bh_mac, bh_target_bssid)
    bh_mac = bh_mac:sub(1,17)
    bh_target_bssid = bh_target_bssid:sub(1,17)
    local r = c_trigger_back_haul_steering_on_agent(bh_mac, bh_target_bssid)
    http.write_json(r)
end

function trigger_wps_fh_agent(fh_bss_mac)
    fh_bss_mac = fh_bss_mac:sub(1,17)
    local r = c_trigger_wps_fh_agent(fh_bss_mac)
    http.write_json(r)
end

function trigger_multi_ap_on_boarding(ifmed)
    assert(ifmed)
    onboardingType = ifmed
    debug_write("trigger_multi_ap_on_boarding: onboardingType:"..ifmed)
    local r = c_trigger_multi_ap_on_boarding(ifmed)
    http.write_json(r)
end

function get_runtime_topology()
    local r = c_get_runtime_topology()
    http.write_json(r)
end

function get_client_capabilities()
    local r = c_get_client_capabilities()
    http.write_json(r)
end

function get_bh_connection_status()
    local r = c_get_bh_connection_status()
    http.write_json(r)
end

function get_sta_steering_progress()
    local r = {}
    local fd = io.open("/tmp/sta_steer_progress","r")
    if not fd then
        r["status"] = "Failed to open /tmp/sta_steer_progress file in read mode!"
    else
        r["sta_steering_info"] = fd:read("*all")
        r["status"] = "SUCCESS"
    end
    http.write_json(r)
end

function get_al_mac(devRole)
    local r = mtkwifi.get_easymesh_al_mac(devRole)
    http.write_json(r)
end

function apply_wifi_bh_priority(bhPriority2G, bhPriority5GL, bhPriority5GH, bhPriority6G)
    assert(bhPriority2G)
    assert(bhPriority5GL)
    assert(bhPriority5GH)
    debug_write("apply_wifi_bh_priority:BhPriority2G:"..bhPriority2G..", BhPriority5GL: "..bhPriority5GL..", BhPriority5GH: "..bhPriority5GH..", bhPriority6G: "..bhPriority6G)
    local  _is_support_6G = mtkwifi._is_support_6G()
    if _is_support_6G then _is_support_6G = 1 else _is_support_6G = 0 end
    local r = c_apply_wifi_bh_priority(bhPriority2G, bhPriority5GL, bhPriority5GH, bhPriority6G, _is_support_6G)
    if r.status == "SUCCESS" then
        os.execute("uci set mapd.mapd_cfg.BhPriority2G="..bhPriority2G)
        os.execute("uci set mapd.mapd_cfg.BhPriority5GL="..bhPriority5GL)
        os.execute("uci set mapd.mapd_cfg.BhPriority5GH="..bhPriority5GH)
        os.execute("uci set mapd.mapd_cfg.BhPriority6G="..bhPriority6G)
        mtkwifi.save_read_easymesh_profile()

        os.execute("uci set mapd.mapd_user.BhPriority2G="..bhPriority2G)
        os.execute("uci set mapd.mapd_user.BhPriority5GL="..bhPriority5GL)
        os.execute("uci set mapd.mapd_user.BhPriority5GH="..bhPriority5GH)
        os.execute("uci set mapd.mapd_user.BhPriority6G="..bhPriority6G)
        os.execute("uci commit")
        os.execute("lua /etc/uci2map.lua")
        os.execute("sync >/dev/null 2>&1")
    end
    http.write_json(r)
end

function apply_ap_steer_rssi_th(rssi)
    assert(rssi)
    local r = c_apply_ap_steer_rssi_th(rssi)
    if r.status == "SUCCESS" then
        local APSteerRssiTh = mtkwifi.read_pipe("uci get mapd.mapd_cfg.APSteerRssiTh") or ""
        if APSteerRssiTh ~= rssi then
            os.execute("uci set mapd.mapd_user.APSteerRssiTh="..rssi)
            os.execute("uci set mapd.mapd_cfg.APSteerRssiTh="..rssi)
            os.execute("uci commit")
        end
        local LowRSSIAPSteerEdge_RE = mtkwifi.read_pipe("uci get mapd.mapd_strng.LowRSSIAPSteerEdge_RE") or ""
        local mapd_rssi = tonumber(rssi) + 94
        if LowRSSIAPSteerEdge_RE ~= mapd_rssi then
            os.execute("uci set mapd.mapd_strng.LowRSSIAPSteerEdge_RE="..mapd_rssi)
            os.execute("uci commit")
        end
        os.execute("lua /etc/uci2map.lua")
        os.execute("sync >/dev/null 2>&1")
    end
    http.write_json(r)
end

function apply_force_ch_switch(agent_almac, channel1, channel2, channel3)
    agent_almac = agent_almac:sub(1,17)

    if channel1 == nil then
        channel1 = 0
    end

    if channel2 == nil then
        channel2 = 0
    end

    if channel3 == nil then
        channel3 = 0
    end

    debug_write("apply_force_ch_switch() enter, agent_almac: "..agent_almac..", channel1:"..channel1..", channel2:"..channel2..", channe3:"..channel3)
    local r = c_apply_force_ch_switch(agent_almac, channel1, channel2, channel3)
    debug_write("apply_force_ch_switch() status: "..r.status)
    http.write_json(r)
end

function apply_user_preferred_channel(channel, band)
    assert(channel)
    local  _is_support_6G = mtkwifi._is_support_6G()
    if _is_support_6G then _is_support_6G = 1 else _is_support_6G = 0 end
    debug_write("apply_user_preferred_channel() enter, channel:"..channel)
    local r = c_apply_user_preferred_channel(channel, band, _is_support_6G)
    debug_write("apply_user_preferred_channel() status: "..r.status)
    http.write_json(r)
end

function trigger_channel_planning_r2(band)
    assert(band)
    local r = c_trigger_channel_planning_r2(band)
    http.write_json(r)
end

function trigger_de_dump(almac)
    assert(almac)
    local r = c_trigger_de_dump(almac)
    http.write_json(r)
end

function get_data_element()
    local r = c_get_data_element()
    http.write_json(r)
end

function trigger_channel_scan(almac, band)
    assert(almac)
    local _is_support_6G = mtkwifi._is_support_6G()
    if _is_support_6G then _is_support_6G = 1 else _is_support_6G = 0 end
    debug_write("trigger_channel_scan() enter, device AlMac:"..almac)
    local r = c_trigger_channel_scan(almac, band, _is_support_6G)
    debug_write("trigger_channel_scan() status: "..r.status)
    http.write_json(r)
end

function get_channel_stats()
    local r = c_get_channel_stats()
    http.write_json(r)
end

function get_channel_planning_score()
    local r = c_get_channel_planning_score()
    http.write_json(r)
end

function apply_channel_utilization_th(channelUtilTh2G, channelUtilTh5GL, channelUtilTh5GH, channelUtilTh6G)
    assert(channelUtilTh2G)
    assert(channelUtilTh5GL)
    assert(channelUtilTh5GH)
    local _is_support_6G = mtkwifi._is_support_6G()
    if _is_support_6G then _is_support_6G = 1 else _is_support_6G = 0 end
    if channelUtilTh6G == 0 or channelUtilTh6G == '' or channelUtilTh6G == nil then
        channelUtilTh6G = 0
    end
    local r = c_apply_channel_utilization_th(channelUtilTh2G, channelUtilTh5GL, channelUtilTh5GH, channelUtilTh6G, _is_support_6G)
    if r.status == "SUCCESS" then
        local CUOverloadTh_2G_mapd_cfg = mtkwifi.read_pipe("uci get mapd.mapd_cfg.CUOverloadTh_2G") or ""
        local CUOverloadTh_5G_L_mapd_cfg = mtkwifi.read_pipe("uci get mapd.mapd_cfg.CUOverloadTh_5G_L") or ""
        local CUOverloadTh_5G_H_mapd_cfg = mtkwifi.read_pipe("uci get mapd.mapd_cfg.CUOverloadTh_5G_H") or ""
        local CUOverloadTh_6G_mapd_cfg = mtkwifi.read_pipe("uci get mapd.mapd_cfg.CUOverloadTh_6G") or ""
        if CUOverloadTh_2G_mapd_cfg ~= channelUtilTh2G or
           CUOverloadTh_5G_L_mapd_cfg ~= channelUtilTh5GL or
           CUOverloadTh_5G_H_mapd_cfg ~= channelUtilTh5GH or
           CUOverloadTh_6G_mapd_cfg ~= channelUtilTh6G then
            os.execute("uci set mapd.mapd_user.CUOverloadTh_2G="..channelUtilTh2G)
            os.execute("uci set mapd.mapd_user.CUOverloadTh_5G_L="..channelUtilTh5GL)
            os.execute("uci set mapd.mapd_user.CUOverloadTh_5G_H="..channelUtilTh5GH)
            os.execute("uci set mapd.mapd_user.CUOverloadTh_6G="..channelUtilTh6G)

            os.execute("uci set mapd.mapd_cfg.CUOverloadTh_2G="..channelUtilTh2G)
            os.execute("uci set mapd.mapd_cfg.CUOverloadTh_5G_L="..channelUtilTh5GL)
            os.execute("uci set mapd.mapd_cfg.CUOverloadTh_5G_H="..channelUtilTh5GH)
            os.execute("uci set mapd.mapd_cfg.CUOverloadTh_6G="..channelUtilTh6G)
        end

        local CUOverloadTh_2G_mapd_strng = mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_2G") or ""
        local CUOverloadTh_5G_L_mapd_strng = mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_5G_L") or ""
        local CUOverloadTh_5G_H_mapd_strng = mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_5G_H") or ""
        local CUOverloadTh_6G_mapd_strng = mtkwifi.read_pipe("uci get mapd.mapd_strng.CUOverloadTh_6G") or ""
        if CUOverloadTh_2G_mapd_strng ~= channelUtilTh2G or
           CUOverloadTh_5G_L_mapd_strng ~= channelUtilTh5GL or
           CUOverloadTh_5G_H_mapd_strng ~= channelUtilTh5GH or
           CUOverloadTh_6G_mapd_strng ~= channelUtilTh6G then
            os.execute("uci set mapd.mapd_strng.CUOverloadTh_2G="..channelUtilTh2G)
            os.execute("uci set mapd.mapd_strng.CUOverloadTh_5G_L="..channelUtilTh5GL)
            os.execute("uci set mapd.mapd_strng.CUOverloadTh_5G_H="..channelUtilTh5GH)
            os.execute("uci set mapd.mapd_strng.CUOverloadTh_6G="..channelUtilTh6G)
        end
        os.execute("uci commit")
        os.execute("lua /etc/uci2map.lua")
        os.execute("sync >/dev/null 2>&1")
    end
    http.write_json(r)
end

function get_sta_bh_interface()
    local r = mtkwifi.get_easymesh_on_boarded_iface_info()
    http.write_json(r)
end

function get_ap_bh_inf_list()
    local devs = mtkwifi.get_all_devs()
    local r = c_get_ap_bh_inf_list()
    if r.status == "SUCCESS" then
        r['apBhInfListStr'] = ""
        for mac in string.gmatch(r.macList, "(%x%x:%x%x:%x%x:%x%x:%x%x:%x%x);") do
            for _, dev in ipairs(devs) do
                local bssid_without_lf = dev.apcli and dev.apcli.mac_addr:upper():sub(1,17) or ""
                if mac:upper() == bssid_without_lf then
                    r['apBhInfListStr'] = r['apBhInfListStr']..dev.apcli.vifname..';'
                else
                    for _,vif in ipairs(dev.vifs) do
                        bssid_without_lf = vif.__bssid:upper():sub(1,17)
                        if mac:upper() == bssid_without_lf then
                            r['apBhInfListStr'] = r['apBhInfListStr']..vif.vifname..';'
                        end
                    end
                end
            end
        end
    end
    http.write_json(r)
end

function get_ap_fh_inf_list()
    local devs = mtkwifi.get_all_devs()
    local r = c_get_ap_fh_inf_list()
    if r.status == "SUCCESS" then
        r['apFhInfListStr'] = ""
        for mac in string.gmatch(r.macList, "(%x%x:%x%x:%x%x:%x%x:%x%x:%x%x);") do
            for _, dev in ipairs(devs) do
                local bssid_without_lf = dev.apcli and dev.apcli.mac_addr:upper():sub(1,17) or ""
                if mac:upper() == bssid_without_lf then
                    r['apFhInfListStr'] = r['apFhInfListStr']..dev.apcli.vifname..';'
                else
                    for _,vif in ipairs(dev.vifs) do
                        bssid_without_lf = vif.__bssid:upper():sub(1,17)
                        if mac:upper() == bssid_without_lf then
                            r['apFhInfListStr'] = r['apFhInfListStr']..vif.vifname..';'
                        end
                    end
                end
            end
        end
    end
    http.write_json(r)
end

function validate_easymesh_bss(r, cfgs, alMac, band)
    assert(type(r) == 'table')
    assert(type(cfgs) == 'table')
    assert(type(alMac) == 'string')
    assert(type(band) == 'string')
    if not cfgs[alMac] then
        r['status'] = 'SUCCESS'
    elseif not cfgs[alMac][band] then
        r['status'] = 'SUCCESS'
    else
        local numBss = mtkwifi.get_table_length(cfgs[alMac][band])
        if numBss >= 16 then
            r['status'] = 'No more BSS could be added!'
        else
            r['status'] = 'SUCCESS'
        end
    end
end

function validate_add_easymesh_bss_req(alMac, band)
    local r = {}
    alMac = string.upper(alMac)
    local cfgs = mtkwifi.load_easymesh_bss_cfgs()
    if type(alMac) ~= 'string' then
        r["status"]= "Invalid AL-MAC Type "..type(alMac).." !"
    elseif type(band) ~= 'string' then
        r["status"]= "Invalid Band Type "..type(band).." !"
    else
        if type(cfgs) ~= "table" or next(cfgs) == nil then
            cfgs = {}
            cfgs['wildCardAlMacCfgs'] = {}
            cfgs['distinctAlMacCfgs'] = {}
        end
        if alMac == 'FF:FF:FF:FF:FF:FF' then
            validate_easymesh_bss(r, cfgs['wildCardAlMacCfgs'], alMac, band)
        else
            validate_easymesh_bss(r, cfgs['distinctAlMacCfgs'], alMac, band)
        end
    end
    if type(r) ~= 'table' or next(r) == nil then
        r['status'] = "Unexpected Exception in validate_easymesh_bss()!"
    end
    http.write_json(r)
end

function apply_easymesh_bss_cfg(isLocal)
    local r = c_apply_bss_config_renew()
    local easymesh_bss_cfg_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__easymesh_bss_cfgs_path())
    os.execute("cp -f "..mtkwifi.__easymesh_bss_cfgs_path().." "..easymesh_bss_cfg_applied_path)
    if isLocal then
        return r
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap", "easymesh_bss_config_renew"))
    end
end

function update_easymesh_bss(id, bssInfoInp, isEdit)
    assert(type(isEdit) == 'string')
    local iface_id = 1
    if isEdit == "1" then
        iface_id = id
    else
        local shuci = require("shuci")
        local path = "/etc/config/mapd"
        if not mtkwifi.exists(path) then
            return
        end
        local mapd = shuci.decode(path)
        local iface_list = {}
        if mapd.iface then
            for _, iface in pairs(mapd.iface) do
                table.insert(iface_list, iface[".name"])
            end
            for i=1, 64 do
                if (mtkwifi.is_include(i, iface_list)) then
                    iface_id = i
                    break
                end
            end
        end
        os.execute("uci set mapd."..iface_id.."=iface")
    end

    os.execute("uci set mapd."..iface_id..".mac="..string.lower(bssInfoInp['alMac']))
    os.execute("uci set mapd."..iface_id..".radio="..bssInfoInp['band'])    
    local tmpSSID = bssInfoInp['ssid']:gsub("'","\'\\\'\'")
    os.execute('uci set mapd.'..iface_id..'.ssid=\''..tmpSSID..'\'')
    local authMode = bssInfoInp['authMode']:gsub("|","\\|")
    os.execute("uci set mapd."..iface_id..".authmode="..authMode)
    os.execute("uci set mapd."..iface_id..".EncryptType="..bssInfoInp['encType'])
    if bssInfoInp['passPhrase'] ~= "" and bssInfoInp['passPhrase'] ~= nil then
        local tmpPsk = bssInfoInp['passPhrase']:gsub("'","\'\\\'\'")
        os.execute('uci set mapd.'..iface_id..'.PSK=\''..tmpPsk..'\'')
    end
    os.execute("uci set mapd."..iface_id..".bhbss="..bssInfoInp['isBhBssSupported'])
    os.execute("uci set mapd."..iface_id..".fhbss="..bssInfoInp['isFhBssSupported'])
    os.execute("uci set mapd."..iface_id..".hidden="..bssInfoInp['isHidden'])
    os.execute("uci set mapd."..iface_id..".vlan="..bssInfoInp['fhVlanId'])
    os.execute("uci set mapd."..iface_id..".pvid="..bssInfoInp['primVlan'])
    os.execute("uci set mapd."..iface_id..".pcp="..bssInfoInp['defPCP'])
    os.execute("uci set mapd."..iface_id..".mld_groupID="..bssInfoInp['mldGroupId'])
    os.execute("uci commit")
end

function easymesh_bss_cfg()
    local bssInfoInp = {}
    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            debug_write("easymesh_bss_cfg: Input BSSINFO are of incorrect type!",k,v)
        elseif string.byte(k) ~= string.byte("_") then
            bssInfoInp[k] = v
        end
    end

    if http.formvalue("__apply") or http.formvalue("__save") then
        local cfgs = mtkwifi.load_easymesh_bss_cfgs()
        if bssInfoInp['primVlan'] ~= "N/A" and bssInfoInp['defPCP'] ~= "N/A" then
            for alMac,alMacTbl in pairs(cfgs['wildCardAlMacCfgs']) do
                for band,bssInfoTbl in pairs(alMacTbl) do
                    for _,bssInfo in pairs(bssInfoTbl) do
                        bssInfo['primVlan'] = "N/A"
                        bssInfo['defPCP'] = "N/A"
                        os.execute("uci set mapd."..bssInfo['id']..".pvid=N/A")
                        os.execute("uci set mapd."..bssInfo['id']..".pcp=N/A")
                    end
                end
            end

            for alMac,alMacTbl in pairs(cfgs['distinctAlMacCfgs']) do
                for band,bssInfoTbl in pairs(alMacTbl) do
                    for _,bssInfo in pairs(bssInfoTbl) do
                        os.execute("uci set mapd."..bssInfo['id']..".pvid=N/A")
                        os.execute("uci set mapd."..bssInfo['id']..".pcp=N/A")
                    end
                end
            end
        end

        update_easymesh_bss(http.formvalue('__EDIT_ID'), bssInfoInp, http.formvalue('__IS_EDIT'))

        mtkwifi.save_easymesh_bss_cfgs()

        if http.formvalue("__apply") then
            apply_easymesh_bss_cfg(true)
        end
        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap", "easymesh_bss_config_renew"))
    elseif http.formvalue("mld_reconfig") then
        local id = http.formvalue('__EDIT_ID')
        if http.formvalue('__IS_EDIT') == "1" then
            os.execute("uci set mapd."..id..".mld_groupID="..bssInfoInp['mldGroupId'])
            os.execute("uci commit")
        end

        mtkwifi.save_easymesh_bss_cfgs()

        local easymesh_bss_cfg_applied_path = mtkwifi.__profile_applied_settings_path(mtkwifi.__easymesh_bss_cfgs_path())
        os.execute("cp -f "..mtkwifi.__easymesh_bss_cfgs_path().." "..easymesh_bss_cfg_applied_path)

        luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap", "easymesh_bss_config_renew"))
        os.execute("1905ctrl dev ap_mlo_link_chg")
    end
end

function remove_easymesh_bss_cfg_req()
    local r = {}
    local cfgs = mtkwifi.load_easymesh_bss_cfgs()
    if type(cfgs) ~= "table" or next(cfgs) == nil then
        r["status"]= mtkwifi.__easymesh_bss_cfgs_path().." file not found!"
    else
        local bssInfoInp = {}
        for k,v in pairs(http.formvalue()) do
            if type(v) ~= type("") and type(v) ~= type(0) then
                r["status"]= "Input BSSINFO are of incorrect type!"
                break
            elseif string.byte(k) ~= string.byte("_") then
                bssInfoInp[k] = v
            end
        end
        r['status'] = "SUCCESS"
        os.execute("uci delete mapd."..bssInfoInp["id"])
        local length = tonumber(mtkwifi.__trim(mtkwifi.read_pipe("tail -n 1 /etc/map/wts_bss_info_config | cut -d ',' -f 1") or ""))
        for start=tonumber(bssInfoInp["id"])+1, length do
            os.execute("uci rename mapd."..start.."="..start-1)
        end
        os.execute("uci commit")
    end
    if type(r) ~= 'table' or next(r) == nil then
        r['status'] = "Unexpected Exception in remove_easymesh_bss()!"
    else
        mtkwifi.save_easymesh_bss_cfgs()
        r = apply_easymesh_bss_cfg(true)
    end
    http.write_json(r)
end

function updata_easymesh_sta_mlo_cfg_req(bstaMac, ruid_or_band, mlo_links)
    local r = {}
    local bstaMloList = mtkwifi.load_easymesh_sta_mlo_cfgs()
    for staMac, bsta in pairs(bstaMloList) do
        if bsta ~= nil then
            os.execute("uci delete mapd."..bsta["name"])
            os.execute("uci commit")
        end
    end

    if #ruid_or_band == 0 then
        bstaMloList[bstaMac] = nil
    else
        if bstaMloList[bstaMac] then
            bstaMloList[bstaMac]['mlo_links'] = mlo_links
            bstaMloList[bstaMac]["ruid_or_band"] = ruid_or_band
        else
            bstaMloList[bstaMac] = {}
            bstaMloList[bstaMac]['mlo_links'] = mlo_links
            bstaMloList[bstaMac]['ruid_or_band'] = ruid_or_band
        end
    end
    lineIdx=0
    for k,staInfo in pairs(bstaMloList) do
        if k ~= "ff:ff:ff:ff:ff:ff" then
            lineIdx = lineIdx+1
            name = "stamld"..lineIdx
            os.execute("uci set mapd."..name.."=bh_sta")
            os.execute("uci set mapd."..name..".almac="..k)
            os.execute("uci set mapd."..name..".mlo_links="..staInfo['mlo_links'])
            os.execute("uci set mapd."..name..".ruid_or_band='"..staInfo['ruid_or_band'].."'")
        end
    end

    for k,staInfo in pairs(bstaMloList) do
        if k == "ff:ff:ff:ff:ff:ff" then
            lineIdx = lineIdx+1
            name = "stamld"..lineIdx
            os.execute("uci set mapd."..name.."=bh_sta")
            os.execute("uci set mapd."..name..".almac="..k)
            os.execute("uci set mapd."..name..".mlo_links="..staInfo['mlo_links'])
            os.execute("uci set mapd."..name..".ruid_or_band='"..staInfo['ruid_or_band'].."'")
        end
    end
    os.execute("uci commit")
    mtkwifi.save_easymesh_bss_cfgs()

    r['status'] = "Success"
    http.write_json(r)

    os.execute("1905ctrl dev bsta_mlo_link_chg")
end

function get_user_preferred_channel()
    local _is_support_6G = mtkwifi._is_support_6G()
    if _is_support_6G then _is_support_6G = 1 else _is_support_6G = 0 end
    local r = c_get_user_preferred_channel(_is_support_6G)
    http.write_json(r)
end

function get_sp_rule_list()
    local r = c_get_sp_rule_list()
    http.write_json(r)
end

function del_sp_rule(index)
    if index == nil then
        index = ""
    end
    local r = c_del_sp_rule(index)
    http.write_json(r)
end

function sp_rule_reorder(index1, index2)
    local r = c_sp_rule_reorder(index1, index2)
    http.write_json(r)
end

function sp_rule_move(index, action)
    local r = c_sp_rule_move(index, action)
    http.write_json(r)
end

function sp_rule_add(str_rule)
    str_rule = string.gsub(str_rule, "] ", "]+")
    local r = c_sp_rule_add(str_rule)
    http.write_json(r)
end

function sp_config_done()
    local r = c_sp_config_done()
    http.write_json(r)
end

function get_qos_rule_list(cfgtype)
    local r = c_get_qos_rule_list(cfgtype)
    http.write_json(r)
end

function qos_config_done()
    local r = c_qos_config_done()
    http.write_json(r)
end

function qos_config_change(str_rule)
    local r = c_qos_config_change(str_rule)
    http.write_json(r)
end

function submit_dpp_uri()
    uri = http.formvalue("uri")
    os.execute("wappctrl ra0 dpp dpp_qr_code ".."\""..uri.."\"")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap"))
end

function start_dpp_onboarding()
    os.execute("wappctrl ra0 dpp dpp_start")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap"))
end

function generate_dpp_uri()
    os.execute("wappctrl ra0 dpp dpp_bootstrap_gen type=qrcode")
    luci.http.redirect(luci.dispatcher.build_url("admin", "mtk", "multi_ap"))
end

function retrive_dpp_uri()
    local result = mtkwifi.read_pipe(tostring("mapd_cli /tmp/mapd_ctrl get_dpp_uri").." 2>&1")
    result = result:gsub("<", "&lt;")
    http.write(tostring(result))
end
