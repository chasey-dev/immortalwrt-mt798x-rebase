# BE72PRO WEXT 固件

用于锐捷 BE72PRO U-Boot Mod，RTL837x GSW，`ubi-232.5`。系统基于 ImmortalWrt 25.12-SNAPSHOT、Linux 6.12.116。默认开启 2.4 GHz 和 5 GHz，两个频段都不设密码。

## 源码版本

- BE72PRO 移植底包提交为 `5dae849eea`，上游 `25.12-dev-wifi7` 提交为 `dca94685cd73309b9902b1976aab4b2deccf86eb`。
- 旧 MTWiFi 驱动来自锐捷 `mt798x-mt799x-mtwifi_be72pro_be68u` 分支，固定为 `401e2ba1682314e48aabaa1af28d0e161aa58554`。构建时会下载该提交的源代码归档并校验 SHA-256。
- `feeds.lock` 固定 OpenWrt 软件包及 LuCI 的 feeds 提交。目标构建配置位于 `defconfig/low-mem-512m/mt7988-mt7992-be72pro-gsw-wext.config`。

## 编译

在 ARM64 Ubuntu 的 OpenWrt 构建环境中，从仓库根目录执行：

```sh
bash scripts/be72pro-wext/build.sh
```

需要常规 OpenWrt 构建工具，并安装 `python3`、`rsync`、`patch`、`curl` 和 `squashfs-tools`。脚本会按锁定版本准备 feeds，构建带 WEXT 的底包，用同一套内核和工具链编译六个旧版 MTWiFi 模块，再生成最终升级包。可用 `JOBS` 调整 OpenWrt 编译并行度。若已有锁定提交的锐捷源码，可通过 `BE72PRO_VENDOR_TREE` 指定目录。

最终升级包位于 `bin/targets/mediatek/filogic/be72pro-wext/`。该目录中的 `*-wext-wifi-on-sysupgrade.bin` 使用旧版锐捷驱动和 PCI 模式，WARP/WED 关闭。

## 主要改动

- `target/linux/mediatek/` 下包含 WEXT 配置、SPI NAND 的 WP/HOLD 复用、Wi-Fi 复位极性和 ramoops 设备树修改。
- `package/network/utils/iwinfo/` 下包含锐捷 `mtk` 后端及 6.12 WEXT 数据结构适配。`package/system/rpcd/` 下的补丁增加 `mtk` 的无线设备查询。
- `legacy-root/`、`firmware-map.json` 和 `ruijie-legacy-mtwifi.*` 记录了最终镜像中使用的 DAT、加载参数、厂商射频固件和 netifd 适配。

此前交付的升级包 SHA-256 为 `34f40ca0e63ca29401af8d9b1f3e4687230b1485d8be3840af3be5b00f730470`。重新构建会生成对应的 `SHA256SUMS`。
