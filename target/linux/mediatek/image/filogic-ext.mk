define Device/clx_s20p
  DEVICE_VENDOR := CLX
  DEVICE_MODEL := S20P
  DEVICE_DTS := mt7986a-clx-s20p
  DEVICE_DTS_DIR := ../dts-ext
  DEVICE_PACKAGES := kmod-usb3 automount f2fsck mkf2fs
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += clx_s20p

define Device/cudy_tr3000-v1-mtkuboot
  DEVICE_VENDOR := Cudy
  DEVICE_MODEL := TR3000
  DEVICE_VARIANT := v1 (MTK U-Boot layout)
  DEVICE_DTS := mt7981b-cudy-tr3000-v1-mtkuboot
  DEVICE_DTS_DIR := ../dts-ext
  SUPPORTED_DEVICES += R47
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_PACKAGES := kmod-usb3 automount
endef
TARGET_DEVICES += cudy_tr3000-v1-mtkuboot

define Device/h3c_magic-nx30-pro-mtkuboot
  DEVICE_VENDOR := H3C
  DEVICE_MODEL := Magic NX30 Pro
  DEVICE_VARIANT := (MTK U-Boot layout)
  DEVICE_DTS := mt7981b-h3c-magic-nx30-pro-mtkuboot
  DEVICE_DTS_DIR := ../dts-ext
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += h3c_magic-nx30-pro-mtkuboot

define Device/netcore_n60-pro-mtkuboot
  DEVICE_VENDOR := Netcore
  DEVICE_MODEL := N60 Pro
  DEVICE_VARIANT := (MTK U-Boot layout)
  DEVICE_DTS := mt7986a-netcore-n60-pro-mtkuboot
  DEVICE_DTS_DIR := ../dts-ext
  DEVICE_PACKAGES := kmod-usb3 automount kmod-usb-ledtrig-usbport 
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += netcore_n60-pro-mtkuboot

define Device/xiaomi_mi-router-ax3000t-mtkuboot
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router AX3000T
  DEVICE_VARIANT := (MTK U-Boot layout)
  DEVICE_DTS := mt7981b-xiaomi-mi-router-ax3000t-mtkuboot
  DEVICE_DTS_DIR := ../dts-ext
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-ax3000t-mtkuboot

define Device/xiaomi_mi-router-wr30u-mtkuboot
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router WR30U
  DEVICE_VARIANT := (MTK U-Boot layout)
  DEVICE_DTS := mt7981b-xiaomi-mi-router-wr30u-mtkuboot
  DEVICE_DTS_DIR := ../dts-ext
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-wr30u-mtkuboot

define Device/wirelesstag_zx7981pd-ubootmod
  DEVICE_VENDOR := Wireless-Tag
  DEVICE_MODEL := ZX7981PD
  DEVICE_VARIANT := (OpenWrt U-Boot layout)
  DEVICE_DTS := mt7981b-wirelesstag-zx7981pd-ubootmod
  DEVICE_DTS_DIR := ../dts-ext
  DEVICE_PACKAGES := kmod-usb3
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  UBOOTENV_IN_UBI := 1
  KERNEL := kernel-bin | lzma
  KERNEL_INITRAMFS := kernel-bin | lzma | \
        fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | \
        fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | append-metadata
  ARTIFACTS := preloader.bin bl31-uboot.fip
  ARTIFACT/preloader.bin := mt7981-bl2 spim-nand-ddr3
  ARTIFACT/bl31-uboot.fip := mt7981-bl31-uboot wirelesstag_zx7981pd
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
  ARTIFACTS += initramfs-factory.ubi
  ARTIFACT/initramfs-factory.ubi := append-image-stage initramfs-recovery.itb | ubinize-kernel
endif
endef
TARGET_DEVICES += wirelesstag_zx7981pd-ubootmod
