# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [1d34e7b](https://github.com/immortalwrt/immortalwrt/commit/1d34e7b88708d4eeb3feabe0b2b6f835a909c9c0)

```
mediatek: fix merge conflict

Fixes: #2458

Fixes: 3a0e732472ba ("Merge Official Source")
Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [511100a](https://github.com/mediatek/mtk-openwrt-feeds/commit/511100a886cf99a12588ccbb810c70928a772027)

```
[openwrt-25.12][mt7988][npu][Enable NPU L4S in autobuild defconfig]

[Description]
Enable NPU package and L4S support in mt798x_rfb autobuild defconfig:
1. Add CONFIG_PACKAGE_kmod-npu=y
2. Add CONFIG_MTK_NPU_L4S=y
for both mt7992 and mt7996 25.12 profiles.

[Info to Customer]
N/A

Change-Id: I124b7f93a7c068ac87cd35343039470276baaf5e
```

### l1parser: [081bb31](https://github.com/chasey-dev/l1parser/commit/081bb31211efc74594d25bfd1bb5811f3408a205)

```
feat(ucode): add get all device map support
```
## About External Devices HNAT
> [!WARNING]
> Current HNAT support for external devices is basic and lack of complete test for various types. Please use with caution.

> [!IMPORTANT]
> Please keep interface `rxppd` in your bridge device (e.g. `br-lan`) while using external device HNAT.

### Support Matrix:
|               |  Ext as WAN   | Ext as LAN                |
|   :----:      |   :----:      | :----:                    |
|  **Ethernet** |      ✔️       |   ❌                     |
| **AP/ApCli**  |      ✔️       |   ⚠️(**Untested**)       |

## Acknowledgements
HNAT support for external devices is adapted from [Padavanonly's repo](https://github.com/padavanonly/immortalwrt-mt798x-6.6). 