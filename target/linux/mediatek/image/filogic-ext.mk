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
