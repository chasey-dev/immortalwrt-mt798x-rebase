#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtwifi-cfg-ucode
PKG_LICENSE:=GPL-2.0
PKG_VERSION:=3
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

UCODE_DEPENDS:= \
  +ucode \
  +ucode-mod-fs \
  +ucode-mod-log \
  +ucode-mod-ubus \
  +ucode-mod-uci \
  +ucode-mod-uloop \
  +ucode-mod-datconf \
  +ucode-mod-l1parser

define Package/mtwifi-cfg-ucode/Default
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=mtwifi configure scripts for mt798x (ucode)
  CONFLICTS:=mtwifi-cfg
endef

define Package/mtwifi-cfg-ucode/description
  New generation mtwifi configuration scripts powered by ucode.
  Replaces the old Lua implementation.
endef

define Package/mtwifi-cfg-ucode
  $(call Package/mtwifi-cfg-ucode/Default)
  DEPENDS:=$(UCODE_DEPENDS) +wifi-scripts +iwinfo-ucode
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/mtwifi-cfg-ucode/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,mtwifi-cfg-ucode))