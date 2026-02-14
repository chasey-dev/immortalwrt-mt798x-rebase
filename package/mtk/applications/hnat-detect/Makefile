#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hnat-detect
PKG_LICENSE:=GPL-2.0
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

UCODE_DEPENDS:= \
  +ucode \
  +ucode-mod-fs \
  +ucode-mod-log \
  +ucode-mod-uci

define Package/hnat-detect/Default
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=HNAT detect utility
endef

define Package/hnat-detect/description
  OpenWrt fw4 based detect utility to configure ppd/wan/lan of HNAT driver.
endef

define Package/hnat-detect
  $(call Package/hnat-detect/Default)
  DEPENDS:=$(UCODE_DEPENDS) +kmod-dummy
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/hnat-detect/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,hnat-detect))
