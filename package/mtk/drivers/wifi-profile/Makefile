#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=wifi-profile
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/wifi-profile
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  DEPENDS:=+WIFI_SCRIPT_SUPPORT:datconf-lua +WIFI_SCRIPT_SUPPORT:lua +WIFI_SCRIPT_SUPPORT:libuci-lua
  TITLE:=Build WiFi l1profile.data on demand & install wifi l2 profile.
  VERSION:=$(PKG_RELEASE)
  MENU:=1
endef

define Package/wifi-profile/description
  This package helps to build l1profile on demand & install wifi l2 profile.
endef

define Package/wifi-profile/config
	if PACKAGE_wifi-profile
	source "$(SOURCE)/l1profile.config.in"
	endif
endef


CFG_PREFIX:=CONFIG_MTK_WLAN
define Build/Compile
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	python ./make-l1profile.py $(TOPDIR)/.config /dev/null
	if [ "$$(CONFIG_PACKAGE_kmod-unified_wlan)" = "y" ] ; then \
		grep "$(CFG_PREFIX)" $(TOPDIR)/.config > $(PKG_BUILD_DIR)/wlan_kconfig; \
		sed -i '/# $(CFG_PREFIX)/d' $(PKG_BUILD_DIR)/wlan_kconfig; \
		sed -i -e 's/=.*//g' $(PKG_BUILD_DIR)/wlan_kconfig; \
		sed -i -e 's/$(CFG_PREFIX)_//g' $(PKG_BUILD_DIR)/wlan_kconfig; \
	fi
endef

define Package/wifi-profile/install
	$(INSTALL_DIR) $(1)/etc/wireless/
ifneq ($(CONFIG_MTK_WIFI7_PROFILING_SUPPORT),y)
	python ./make-l1profile.py $(TOPDIR)/.config $(1)/etc/wireless/l1profile.dat > /dev/null
endif

	$(INSTALL_DIR) $(1)/etc/wireless/mediatek/
	echo $(PKG_VERSION) > $(1)/etc/wireless/mediatek/version
	if [ "$$(CONFIG_WIFI_SCRIPT_SUPPORT)" = "y" ]; then \
		$(INSTALL_DIR) $(1)/sbin/; \
		$(INSTALL_DIR) $(1)/lib/wifi/; \
		if [ "$$(CONFIG_MTK_WIFI7_UNIFIED_COMMAND)" = "y" ]; then \
			$(INSTALL_BIN) ./files/unified_script/wifi $(1)/sbin/; \
			$(INSTALL_BIN) ./files/unified_script/mtwifi.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/mtkdat.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/wifi_services.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/hostapd.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/hostapd_wps_er_action.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/supplicant.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/set_bcn.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/set_mgmt_rx.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/unified_script/supplicant_scan_action.sh $(1)/lib/wifi/; \
			if [ "$$(CONFIG_WIFI_QUICK_SETTING)" = "y" ] ; then \
				$(INSTALL_BIN) ./files/unified_script/quick_setting.lua $(1)/lib/wifi/; \
			fi; \
		else \
			$(INSTALL_BIN) ./files/common/wifi_jedi $(1)/sbin/wifi; \
			$(INSTALL_BIN) ./files/common/mtwifi.lua $(1)/lib/wifi/; \
			$(INSTALL_BIN) ./files/common/wifi_services.lua $(1)/lib/wifi/; \
			if [ "$$(CONFIG_WIFI_QUICK_SETTING)" = "y" ] ; then \
			$(INSTALL_BIN) ./files/common/quick_setting.lua $(1)/lib/wifi/; \
			fi; \
		fi; \
	fi

	# Example combination mt7622 + mt7615e/a + mt7615e/a.
	if [ "$$(CONFIG_first_card_name)" = "MT7622" ] ; then \
		$(INSTALL_BIN) ./files/mt7622.1.dat $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
			$(INSTALL_BIN) ./files/mt7622_map.1.dat $(1)/etc/wireless/mediatek/mt7622.1.dat; \
		fi; \
		if [ "$$(CONFIG_second_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.1.dat $(1)/etc/wireless/mediatek/; \
			if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
				$(INSTALL_BIN) ./files/mt7615e_map.1.dat $(1)/etc/wireless/mediatek/mt7615e.1.dat; \
			fi; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b1.5g.dat; \
		fi; \
		if [ "$$(CONFIG_third_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.2.dat $(1)/etc/wireless/mediatek/; \
			if [ "$$(CONFIG_MTK_MAP_SUPPORT)" = "y"  ] ; then \
				$(INSTALL_BIN) ./files/mt7615e_map.2.dat $(1)/etc/wireless/mediatek/mt7615e.2.dat; \
			fi; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b1.5g.dat; \
		fi; \
		$(INSTALL_BIN) ./files/mt7615e*.bin $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7623 + mt7615e/a + mt7615e/a.
	if [ "$$(CONFIG_first_card_name)" = "MT7615" ] ; then \
		$(INSTALL_BIN) ./files/mt7615e.1.dat $(1)/etc/wireless/mediatek/; \
		$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b0.5g.dat; \
		$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.1.b1.5g.dat; \
		if [ "$$(CONFIG_second_card_name)" = "MT7615" ] ; then \
			$(INSTALL_BIN) ./files/mt7615e.2.dat $(1)/etc/wireless/mediatek/; \
			$(INSTALL_BIN) ./files/mt7615a.b0.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b0.5g.dat; \
			$(INSTALL_BIN) ./files/mt7615a.b1.5g.dat $(1)/etc/wireless/mediatek/mt7615a.2.b1.5g.dat; \
		fi; \
		$(INSTALL_BIN) ./files/mt7615e*.bin $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + axe.
	if [ "$$(CONFIG_first_card_name)" = "AXE" ] ; then \
		$(INSTALL_BIN) ./files/axe.1.dat $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + harrier.
	if [ "$$(CONFIG_first_card_name)" = "MT7915" -o "$$(CONFIG_second_card_name)" = "MT7915" \
		-o "$$(CONFIG_third_card_name)" = "MT7915" ] ; then \
		sed -i '/EEPROM_def_bin_path/d' $(1)/etc/wireless/l1profile.dat; \
		$(INSTALL_BIN) ./files/mt7915/mt7915.*.dat $(1)/etc/wireless/mediatek/; \
		$(INSTALL_BIN) ./files/mt7915/wifi_cert.*.dat $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + Panther.
	if [ "$$(CONFIG_first_card_name)" = "MT7986" -o "$$(CONFIG_second_card_name)" = "MT7986" ] ; then \
		sed -i '/EEPROM_def_bin_path/d' $(1)/etc/wireless/l1profile.dat; \
		$(INSTALL_BIN) ./files/mt7986/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + Merlin.
	if [ "$$(CONFIG_first_card_name)" = "MT7916" -o "$$(CONFIG_second_card_name)" = "MT7916" ] ; then \
		sed -i '/EEPROM_def_bin_path/d' $(1)/etc/wireless/l1profile.dat; \
		$(INSTALL_BIN) ./files/mt7916/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + Cheetah.
	if [ "$$(CONFIG_first_card_name)" = "MT7981" -o "$$(CONFIG_second_card_name)" = "MT7981" ] ; then \
		sed -i '/EEPROM_def_bin_path/d' $(1)/etc/wireless/l1profile.dat; \
		$(INSTALL_BIN) ./files/mt7981/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + bellwether.
	if [ "$$(CONFIG_first_card_name)" = "Bellwether" -o "$$(CONFIG_second_card_name)" = "Bellwether" ] ; then \
		sed -i '/EEPROM_def_bin_path/d' $(1)/etc/wireless/l1profile.dat ; \
		$(INSTALL_BIN) ./files/bellwether/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7622 + Eagle.
ifneq ($(CONFIG_MTK_WIFI7_PROFILING_SUPPORT),y)
	if [ "$$(CONFIG_first_card_name)" = "MT7990" -o "$$(CONFIG_second_card_name)" = "MT7990" ] ; then \
		$(INSTALL_BIN) ./files/mt7990/* $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_WIFI7_SKU_TYPE)" = "BE13000_255" ] ; then \
			$(CP) ./files/mt7990/mt7990.b1.5g.dat $(1)/etc/wireless/mediatek/mt7990.b1.dat; \
			$(CP) ./files/mt7990/mt7990.b2.5g.dat $(1)/etc/wireless/mediatek/mt7990.b2.dat; \
		fi; \
		if [ "$$(CONFIG_MTK_WIFI7_2ADIE_TRIBAND)" = "y" ] ; then \
			sed -i 's/DfsZeroWait=0/DfsZeroWait=1/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
			sed -i 's/DfsDedicatedZeroWait=1/DfsDedicatedZeroWait=0/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
			sed -i 's/DfsZeroWaitDefault=1/DfsZeroWaitDefault=0/g' $(1)/etc/wireless/mediatek/mt7990.b1.dat ; \
		fi; \
	fi

	# Example combination mt7622 + Kite.
	if [ "$$(CONFIG_first_card_name)" = "MT7992" -o "$$(CONFIG_second_card_name)" = "MT7992" ] ; then \
		$(INSTALL_BIN) ./files/mt7992/* $(1)/etc/wireless/mediatek/; \
		if [ "$$(CONFIG_MTK_WIFI7_SKU_TYPE)" = "BE3600SDB" ] ; then \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_3.dat ; \
			sed -i "s/mt7992.1_1.dat/mt7992.1_3.dat/" $(1)/etc/wireless/l1profile_3.dat; \
			sed -i "s/EEPROM_offset="0x0"/EEPROM_offset="0x4000"/" $(1)/etc/wireless/l1profile_3.dat; \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_2.dat ; \
			sed -i "s/mt7992.1_1.dat/mt7992.1_2.dat/" $(1)/etc/wireless/l1profile_2.dat; \
			sed -i "s/EEPROM_offset="0x0"/EEPROM_offset="0x2000"/" $(1)/etc/wireless/l1profile_2.dat; \
			cp $(1)/etc/wireless/l1profile.dat $(1)/etc/wireless/l1profile_1.dat ; \
		fi; \
	fi

	# Example combination mt7988 + Griffin.
	if [ "$$(CONFIG_first_card_name)" = "MT7993" -o "$$(CONFIG_second_card_name)" = "MT7993" ] ; then \
		$(INSTALL_BIN) ./files/mt7993/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example combination mt7988 + Blackhawk_A0.
	if [ "$$(CONFIG_first_card_name)" = "BLACKHAWK" -o "$$(CONFIG_second_card_name)" = "BLACKHAWK" ] ; then \
		$(INSTALL_BIN) ./files/blackhawk/* $(1)/etc/wireless/mediatek/; \
	fi
endif

	# Example mt7902
	if [ "$$(CONFIG_first_card_name)" = "MT7902" -o "$$(CONFIG_second_card_name)" = "MT7902" ] ; then \
		 $(INSTALL_BIN) ./files/bellwether/* $(1)/etc/wireless/mediatek/; \
	fi

	# Example mt7615D
	if [ "$$(CONFIG_first_card_name)" = "MT7615D" -o "$$(CONFIG_second_card_name)" = "MT7615D" ] ; then \
		$(INSTALL_BIN) ./files/mt7615D/*.* $(1)/etc/wireless/mediatek/; \
	fi

	if [ "$$(CONFIG_PACKAGE_hostapd-common)" != "y" -a "$$(CONFIG_PACKAGE_kmod-unified_wlan)" != "y" ] ; then \
		$(INSTALL_DIR) $(1)/etc/rc.button; \
		$(INSTALL_BIN) ./files/common/wps-hotplug.sh $(1)/etc/rc.button/wps; \
	fi
endef

$(eval $(call BuildPackage,wifi-profile))
