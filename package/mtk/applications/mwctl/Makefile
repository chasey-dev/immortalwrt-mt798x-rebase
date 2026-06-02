include $(TOPDIR)/rules.mk

PKG_NAME:=mwctl
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/kernel.mk

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)
CMAKE_BINARY_DIR:=$(PKG_BUILD_DIR)

define Package/mwctl
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=vendor cmd for logan/mt76 driver
  DEPENDS:=+libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny
TARGET_CFLAGS += -I$(LINUX_DIR)/include/uapi/linux/mtk_nl80211_inc

define Package/mwctl/install
	mkdir -p $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mwctl $(1)/usr/sbin
endef

$(eval $(call BuildPackage,mwctl))
