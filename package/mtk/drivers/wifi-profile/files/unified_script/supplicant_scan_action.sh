#!/bin/sh

iface=$1
event=$2

case $event in
	CTRL-EVENT-SCAN-STARTED) echo "start" > /var/run/scan_state-$iface;;
	CTRL-EVENT-SCAN-FAILED) echo "failed" > /var/run/scan_state-$iface;;
	CTRL-EVENT-SCAN-RESULTS) echo "done" > /var/run/scan_state-$iface;;
esac

