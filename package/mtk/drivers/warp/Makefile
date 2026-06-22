# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=warp
P4REV:=
PKG_SOURCE_DATE:=2025-06-13
PKG_SOURCE_VERSION:=433271
PKG_SOURCE:=warp_20250613-433271.tar.xz
PKG_SOURCE_URL:=https://github.com/chasey-dev/objs/releases/download/v1
PKG_HASH:=6537088695c124135b695bf4ce6d4f9febecd3fb41889c4419de77780dca6488
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

PKG_KCONFIG:= \
	WARP_VERSION \
	WARP_DBG_SUPPORT \
	WED_HW_RRO_SUPPORT \
	MTK_MEMORY_SHRINK \
	WARP_BM_SHRINK_RING \
	WARP_RX_PAGE_BM_NUM \
	WARP_CHIPSET \
	WARP_WO_EMBEDDED_LOAD \
	WARP_MEMORY_LEAK_DBG \
	TARGET_mediatek

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)

define KernelPackage/warp
  CATEGORY:=MTK Properties
  TITLE:=MTK warp driver

  DEPENDS:=@TARGET_mediatek +kmod-mediatek_hnat

  FILES:=$(PKG_BUILD_DIR)/mtk_warp.ko
  AUTOLOAD:=$(call AutoLoad,60,mtk_warp,1)
  SUBMENU:=Drivers
  MENU:=1
endef

define KernelPackage/warp/config
	source "$(SOURCE)/config.in"
endef

KCFLAGS += $(addprefix -Wno-error=,missing-prototypes missing-declarations)


define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		KCFLAGS="$(KCFLAGS)" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		modules
endef

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

MAKE_FLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$c))) 

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

ifeq ($(CONFIG_WED_HW_RRO_SUPPORT), y)
define KernelPackage/warp/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp $(PKG_BUILD_DIR)/bin/*WOCPU*_RAM_CODE_release.bin $(1)/lib/firmware/;
endef
endif

$(eval $(call KernelPackage,warp))


