#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=airoha-phy-firmware
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Mason Chang <mason-cw.chang@mediatek.com>

include $(INCLUDE_DIR)/package.mk

define Package/airoha-phy-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Airoha Ethernet PHY firmware
endef

define Package/airoha-phy-firmware/description
  This is a firmware package for Airoha Ethernet PHY
endef

define Build/Compile

endef

define Package/airoha-phy-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) ./files/an8811hb/EthMD32_CRC.DM.bin $(1)/lib/firmware/
	$(INSTALL_DATA) ./files/an8811hb/EthMD32_CRC.DSP.bin $(1)/lib/firmware/
endef
$(eval $(call BuildPackage,airoha-phy-firmware))
