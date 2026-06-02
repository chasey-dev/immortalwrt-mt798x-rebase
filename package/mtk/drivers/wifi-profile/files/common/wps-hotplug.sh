#!/bin/sh

if [ -f /etc/wireless/l1profile.dat ]; then
	DAT_PATH=/etc/wireless/l1profile.dat
fi

if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	echo "WPS Button Pressed........." > /dev/console
	wifi1_ifname=`cat $DAT_PATH | grep -r INDEX0_main_ifname | awk -F = '{printf $2}'`
	iwpriv $wifi1_ifname set WscConfMode=4
	iwpriv $wifi1_ifname set WscMode=2
	iwpriv $wifi1_ifname set WscConfStatus=2
	iwpriv $wifi1_ifname set WscGetConf=1
fi

return 0
