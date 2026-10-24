# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dsa-rtl837x
PKG_VERSION:=2026.10.05
PKG_RELEASE:=1

# Latest upstream verified on 2026-10-23; pin it for reproducible builds.
PKG_SOURCE_VERSION:=b14546dbeabf500c287a6ff7ce847919a02d4a26
PKG_SOURCE:=rtl837x-dsa-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/airjinkela/rtl837x-dsa-driver/tar.gz/
PKG_SOURCE_URL_FILE:=$(PKG_SOURCE_VERSION)
PKG_HASH:=900d052ec9cc15e8ff815f3cb33b7531ef24b980e86d962fc1e61ab9ac6264a0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/rtl837x-dsa-driver-$(PKG_SOURCE_VERSION)

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

# MTK backports can change this ABI without changing LINUX_VERSION_CODE.
RTL837X_COMPAT_CFLAGS:=$(if $(shell test -f "$(LINUX_DIR)/include/linux/phylink.h" && awk '/\(\*pcs_get_state\)/ { capture=1 } capture { print; if ($$0 ~ /;/) exit }' "$(LINUX_DIR)/include/linux/phylink.h" | grep -q neg_mode && echo yes),-DRTL837X_PCS_GET_STATE_HAS_NEG_MODE)

define KernelPackage/dsa-rtl837x
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL837x DSA switch support
  DEPENDS:=@TARGET_mediatek_filogic +kmod-sfp
  KCONFIG:= \
	CONFIG_NEW_LEDS=y \
	CONFIG_LEDS_CLASS=y \
	CONFIG_LEDS_TRIGGERS=y \
	CONFIG_LEDS_TRIGGER_NETDEV=y \
	CONFIG_NET_DSA_TAG_RTL8_4=y \
	CONFIG_NET_DSA_TAG_MXL862_8021Q=y
  FILES:=$(PKG_BUILD_DIR)/src/rtl8372n_dsa.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8372n_dsa,1)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_RTL8372N_DSA=m \
		CONFIG_RTL837X_PHY_PATCH=n \
		CONFIG_RTL8372N_DSA_DEBUG=n \
		CFLAGS_rtl8372n.o="$(RTL837X_COMPAT_CFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,dsa-rtl837x))
