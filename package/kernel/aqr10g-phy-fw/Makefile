#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aqr10g-phy-firmware
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Bo-Cun Chen <bc-bocun.chen@mediatek.com>

include $(INCLUDE_DIR)/package.mk

define Package/aqr10g-phy-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Aquantia 10G Ethernet PHY firmware
endef

define Package/aqr10g-phy-firmware/description
  This is a firmware package for the Aquantia 10G Ethernet PHY
endef

define Build/Compile

endef

define Package/aqr10g-phy-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_DATA) \
		./files/Rhe-05.06-Candidate9-AQR_Mediatek_23B_P5_ID45824_LCLVER1.cld \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		./files/AQR-G4_v5.7.0-AQR_EVB_Generic_X3410_StdCfg_MDISwap_USX_ID46316_VER2148.cld \
		$(1)/lib/firmware/
endef
$(eval $(call BuildPackage,aqr10g-phy-firmware))
