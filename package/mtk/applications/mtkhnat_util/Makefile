# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021 MediaTek Inc.
#
# MTK-factory read and write
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtkhnat_util
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/mtkhnat_util
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=mtk hnat utility
  VERSION:=$(PKG_VERSION)-r$(PKG_RELEASE)
  SUBMENU:=Applications
endef

define Package/mtkhnat_util/description
  mtk hnat util to init hnat module
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/mtkhnat_util/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults

	$(INSTALL_BIN) ./files/mtkhnat $(1)/sbin/
	$(INSTALL_BIN) ./files/mtkhnat.config $(1)/etc/config/mtkhnat
	$(INSTALL_BIN) ./files/mtkhnat.init $(1)/etc/init.d/mtkhnat
endef

$(eval $(call BuildPackage,mtkhnat_util))
