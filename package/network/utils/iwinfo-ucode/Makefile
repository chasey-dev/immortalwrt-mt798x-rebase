include $(TOPDIR)/rules.mk

PKG_NAME:=iwinfo-ucode
PKG_RELEASE:=1
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/package.mk

define Package/ucode-mod-iwinfo
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=ucode binding for libiwinfo
  DEPENDS:=+libucode +libiwinfo
endef

define Package/ucode-mod-iwinfo/description
  Dynamically linked ucode binding for libiwinfo.
  It automatically detects and exposes all backends (nl80211, mtk, etc.)
  supported by the system's libiwinfo.so without needing compile-time flags.
endef

define Package/iwinfo-ucode
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Wireless Information utility rewritten in ucode
  DEPENDS:=+ucode-mod-iwinfo @!PACKAGE_iwinfo
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(FPIC) \
		-Wall -ffunction-sections -Wl,--gc-sections -shared -Wl,--no-as-needed \
		-o $(PKG_BUILD_DIR)/iwinfo_ucode.so \
		$(PKG_BUILD_DIR)/iwinfo_ucode.c \
		-liwinfo
endef

define Package/ucode-mod-iwinfo/install
	$(INSTALL_DIR) $(1)/usr/lib/ucode
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo_ucode.so $(1)/usr/lib/ucode/iwinfo.so
endef

define Package/iwinfo-ucode/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,ucode-mod-iwinfo))
$(eval $(call BuildPackage,iwinfo-ucode))