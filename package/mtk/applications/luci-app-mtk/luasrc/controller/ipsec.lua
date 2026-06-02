local luci  = {}
luci.util = require "luci.util"
luci.http = require "luci.http"
local uci = require "luci.model.uci"

module("luci.controller.ipsec", package.seeall)

function index()
        entry({"admin", "network", "ipsec"}, cbi("ipsec"), _("IP Security"))
        entry({"admin", "network", "ipsec", "vpn_status"}, call("ipsec_vpn_status"), nil).leaf = true
        entry({"admin", "network", "ipsec", "vpn_connect"}, call("ipsec_vpn_connect"), nil).leaf = true
        entry({"admin", "network", "ipsec", "vpn_disconnect"}, call("ipsec_vpn_disconnect"), nil).leaf = true
end

function ipsec_vpn_status()
    local handle = io.popen("ip x s  2>/dev/null")
    local result = handle:read("*all")
    handle:close()
    local obj ={}

    luci.http.prepare_content("application/json")
    if result == "" then
        obj.status = "Disconnected"
        obj.msg = "Disconnected"
        luci.http.write_json(obj)
        return
    end

    local curs = uci.cursor()
    gateway = curs:get("ipsec", "TEST", "gateway")
    if string.find(result, gateway) then
        obj.status = "Connected"
        obj.msg =  "Connected"
        luci.http.write_json(obj)
        return
    else
        obj.status = "Disconnected"
        obj.msg =  "Disconnected"
        luci.http.write_json(obj)
        return
    end
end

function ipsec_vpn_connect()
        local l_gw_name = ""
        local curs = uci.cursor()
        curs:foreach("ipsec", "remote", function(s) l_gw_name = s[".name"] end)
        l_subnet = curs:get("ipsec", "TUNNEL", "local_subnet")
        l_wan    = curs:get("network","wan" ,"device")
        l_mode =  curs:get("ipsec", "TEST", "mode")
        for _, subnet in pairs(l_subnet) do
            luci.util.execi("iptables -t nat -I POSTROUTING -o "..l_wan.." -s "..subnet.." -j ACCEPT")
        end
        luci.util.execi("service swanctl restart")
        luci.util.execi("swanctl --initiate --child "..string.upper(l_mode))

        ipsec_vpn_status()
end

function ipsec_vpn_disconnect()
        local l_gw_name = ""
        local curs = uci.cursor()
        l_mode =  curs:get("ipsec", "TEST", "mode")
        luci.util.execi("swanctl --terminate --child "..string.upper(l_mode))
        ipsec_vpn_status()
end
