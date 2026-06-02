local mtkdat = require("mtkdat")
local nixio = require("nixio")

local function set_mgmt_rx(devname, value)
    local devs, l1parser = mtkdat.__get_l1dat()

    if devname then
        local dev = devs.devname_ridx[devname]
        if dev then
            nixio.syslog("debug", "mwctl "..dev.main_ifname.." set mgmt_rx " .. value)
            os.execute("mwctl "..dev.main_ifname.." set mgmt_rx " .. value)
        end
    else
        for _devname, _dev in mtkdat.spairs(devs.devname_ridx) do
            local dev = devs.devname_ridx[_devname]
            if dev then
                nixio.syslog("debug", "mwctl "..dev.main_ifname.." set mgmt_rx " .. value)
                os.execute("mwctl "..dev.main_ifname.." set mgmt_rx " .. value)
            end
        end
    end
end

function set_mgmt_rx_accept_all(devname)
    set_mgmt_rx(devname, 0)
end

function set_mgmt_rx_accept_beacon_probrsp(devname)
    set_mgmt_rx(devname, 1)
end
