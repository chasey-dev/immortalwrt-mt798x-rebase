# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt_wifi_cmn
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_SOURCE_DATE:=2025-06-13
PKG_SOURCE_VERSION:=415824
PKG_SOURCE:=mt7993_20250613-415824.tar.xz
PKG_SOURCE_URL:=https://github.com/chasey-dev/objs/releases/download/v1
PKG_HASH:=e2900a672a5151aed85f6711240db1ddcf5eed317fd620485a0ee6aee060cfb4

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt_wifi_cmn
  CATEGORY:=MTK Properties
  TITLE:=MTK wifi common layer
  FILES:=$(PKG_BUILD_DIR)/mt_wifi_cmn.ko
  SUBMENU:=Drivers
  MENU:=1
  AUTOLOAD:=$(call AutoLoad,60,$(if $(CONFIG_MTK_WIFI7_CFG80211_SUPPORT), cfg80211) mt_wifi_cmn)
  DEPENDS:=+MTK_WIFI7_CFG80211_SUPPORT:kmod-cfg80211
endef


ifeq (1,1)
define Build/Prepare
	tar xvf $(DL_DIR)/$(PKG_SOURCE) --overwrite -C $(PKG_BUILD_DIR) mt_wifi/os/linux/mt_linux.c mt_wifi/include/os/mt_linux.h mt_wifi/os/linux/Makefile.mt_wifi_cmn
	mv $(PKG_BUILD_DIR)/mt_wifi/os/linux/mt_linux.c $(PKG_BUILD_DIR)
	mv $(PKG_BUILD_DIR)/mt_wifi/include/os/mt_linux.h $(PKG_BUILD_DIR)
	mv $(PKG_BUILD_DIR)/mt_wifi/os/linux/Makefile.mt_wifi_cmn $(PKG_BUILD_DIR)/Makefile
endef
endif


NOSTDINC_FLAGS :=
ifeq ($(CONFIG_MTK_WIFI7_CFG80211_SUPPORT),y)
NOSTDINC_FLAGS += \
	$(KERNEL_NOSTDINC_FLAGS) \
	-DRT_CFG80211_SUPPORT \
	-DBACKPORT_NOSTDINC \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h
endif
NOSTDINC_FLAGS += -DMT_WIFI_COMMON_SUPPORT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,mt_wifi_cmn))
