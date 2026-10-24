# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8261d
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/RuijieNetworksCommunity/rtl8261-phy-driver.git
PKG_SOURCE_DATE:=2026-04-21
PKG_SOURCE_VERSION:=19fa6fa8347026f35b6cb41a5785b76e0e315b14
PKG_MIRROR_HASH:=a8dcba9ca42920f54bbf1f7e532d6547d6ddd64e2005fa80a20772d38f33cdcc

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/phy-rtl8261d
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8261D 10 Gigabit PHY driver
  DEPENDS:=+kmod-hwmon-core
  FILES:=$(PKG_BUILD_DIR)/src/rtl8261d.ko
  AUTOLOAD:=$(call AutoLoad,18,rtl8261d,1)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_REALTEK_PHY_RTL8261D=m \
		modules
endef

$(eval $(call KernelPackage,phy-rtl8261d))
