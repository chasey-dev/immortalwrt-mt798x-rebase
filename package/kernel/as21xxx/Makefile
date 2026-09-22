#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=as21xxx-firmware
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/as21xxx-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Aeonsemi AS21xxx PHY firmware
endef

define Package/as21xxx-firmware/description
  This is a firmware package for the Aeonsemi AS21xxx PHY
endef

define Build/Compile
    true
endef

define Package/as21xxx-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		./firmware/as21x1x_fw.bin \
		$(1)/lib/firmware/
endef
$(eval $(call BuildPackage,as21xxx-firmware))
