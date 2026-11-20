include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl837x_gsw
PKG_VERSION:=0.0.2
PKG_RELEASE:=1
PKG_MAINTAINER:=air jinkela (air_jinkela@163.com)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Other modules
  TITLE:=$(PKG_NAME)
  FILES:=$(PKG_BUILD_DIR)/rtl837x_gsw.ko
  AUTOLOAD:=$(call AutoLoad,99,rtl837x_gsw)
  DEPENDS:=+kmod-swconfig
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CONFIG_RTL837x_GSW=m \
		CONFIG_RTL837x_GSW_PORT_MIB_FEATURE=y \
		modules
endef

$(eval $(call KernelPackage,rtl837x_gsw))
