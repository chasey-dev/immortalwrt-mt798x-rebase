include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Easy QoS
LUCI_DEPENDS:=+tc +nftables +kmod-sched-core +kmod-ifb +kmod-mediatek_hnat @!PACKAGE_luci-app-eqos

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>
PKG_NAME:=luci-app-eqos-mtk

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
