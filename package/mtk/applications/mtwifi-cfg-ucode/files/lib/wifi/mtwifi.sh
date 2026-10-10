#!/bin/sh

# This is a glue layer to stay compatibility with /sbin/wifi
# which is still using shell to discover wifi drivers
append DRIVERS "mtwifi"

detect_mtwifi() {
    /lib/wifi/mtwifi.uc
}