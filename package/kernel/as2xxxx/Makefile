#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=as2xxxx-firmware
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/as2xxxx-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Aeonsemi AS2xxxx PHY firmware
endef

define Package/as2xxxx-firmware/description
  This is a firmware package for the Aeonsemi AS2xxxx PHY
endef

define Build/Compile
    true
endef

define Package/as2xxxx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		./firmware/as21x1x_fw.bin \
		./firmware/as22x1x_fw.bin \
		$(1)/lib/firmware/
endef
$(eval $(call BuildPackage,as2xxxx-firmware))
