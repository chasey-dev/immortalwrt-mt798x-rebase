--This file is created for check some deamons

local mtkdat = require("mtkdat")
local nixio = require("nixio")

-- wifi service that require to start after wifi up
function wifi_service_misc()
    local mapd_default = mtkdat.load_profile("/etc/map/mapd_default.cfg")
    local mapd_user = mtkdat.load_profile("/etc/map/mapd_user.cfg")
    local first_card_cfgs = mtkdat.load_profile(mtkdat.detect_first_card())
    if mapd_default.mode then
        local eth_mode = mapd_default.mode
        local device_role = mapd_default.DeviceRole
        if mapd_user.mode then
            eth_mode = mapd_user.mode
        end
        if mapd_user.DeviceRole then
            device_role = mapd_user.DeviceRole
        end
        -- 1.Wapp
        if mtkdat.exist("/usr/bin/wapp_openwrt.sh") then
            os.execute("/etc/init.d/wapp start")
        end
        -- 2.EasyMesh
        if mtkdat.exist("/usr/bin/EasyMesh_openwrt.sh") then
            if first_card_cfgs.MapMode == "1" then
                if (eth_mode == "0" and device_role == "1") or eth_mode == "1" then
                    os.execute("/etc/init.d/easymesh start")
                else
                    os.execute("/etc/init.d/easymesh_bridge start")
                end
            elseif first_card_cfgs.MapMode ~= "0" then
                os.execute("/etc/init.d/easymesh start")
            end
        end
    else
        -- 1.Wapp
        if mtkdat.exist("/usr/bin/wapp_openwrt.sh") then
            os.execute("/etc/init.d/wapp start")
        end
        -- 2.EasyMesh
        if mtkdat.exist("/usr/bin/EasyMesh_openwrt.sh") then
            os.execute("/etc/init.d/easymesh start")
        end
    end
end

-- wifi service that require to clean up before wifi down
function wifi_service_misc_clean()
    os.execute("rm -rf /tmp/wapp_ctrl")
    os.execute("killall -15 mapd")
    os.execute("killall -15 wapp")
    os.execute("killall -15 p1905_managerd")
    os.execute("killall -15 bs20")
end
