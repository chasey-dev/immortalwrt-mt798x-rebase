include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Easy QoS
LUCI_DEPENDS:=+tc +nftables +kmod-sched-core +kmod-ifb +kmod-mediatek_hnat +mtkhqos_util @!PACKAGE_luci-app-eqos
LUCI_EXTRA_DEPENDS:=mtkhqos_util (>=1-r2)

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>
PKG_NAME:=luci-app-eqos-mtk

define Package/$(PKG_NAME)/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0

rm -f /tmp/luci-indexcache.*
rm -rf /tmp/luci-modulecache/
/etc/init.d/rpcd reload 2>/dev/null

# dnsmasq.init writes dhcp-script into the generated config file, not the
# command line: xappend() strips the leading "--" and appends to $$CONFIGFILE,
# and procd starts dnsmasq as "dnsmasq -C <conf> -k -x <pid>". So resolve the
# -C argument and look for the option inside that file.
restart_dnsmasq=0
for pid in $$(pidof dnsmasq 2>/dev/null); do
	[ -r "/proc/$$pid/cmdline" ] || continue
	conf=$$(tr '\0' '\n' < "/proc/$$pid/cmdline" | sed -n '/^-C$$/{n;p}')
	if [ -z "$$conf" ] || [ ! -r "$$conf" ] || ! grep -q '^dhcp-script=' "$$conf"; then
		restart_dnsmasq=1
		break
	fi
done

if [ "$$restart_dnsmasq" -eq 1 ]; then
	logger -t eqos "restarting dnsmasq once to activate DHCP hotplug"
	/etc/init.d/dnsmasq restart || {
		logger -t eqos "failed to activate DHCP hotplug"
		exit 1
	}
fi

exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
