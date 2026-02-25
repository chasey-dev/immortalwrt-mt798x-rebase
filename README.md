# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [eab2248](https://github.com/immortalwrt/immortalwrt/commit/eab224867755176235ed1099b45413c1b63cbd57) - OpenWrt 25.12.0-rc5

```
rockchip: remove broken vdd-supply props from the usb hub for photonicat2

See [#2235](https://github.com/immortalwrt/immortalwrt/issues/2235).

Sync dts for u-boot while at it.

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
(cherry picked from commit e61a1d0a590b4c50b255b1791d8fda26ba470c03)
```

### MTK OpenWrt Feeds: [26093b6](https://git01.mediatek.com/plugins/gitiles/openwrt/feeds/mtk-openwrt-feeds/+/26093b6d126807756472b7fec5d53951d353029b)

```
[][Critical][kernel][mt7987][eth][linux-firmware: mediatek: mt7987: Update built-in 2.5Gphy firmware to 2026/02/25, v7.3]

[Description]
Update built-in 2.5Gphy firmware to 2026/02/25, v7.3

Change log (v7.2):
- Correct misidentification of TX Pause frame in PHY
Change log (v7.3):
- Fine tune control flow for GBE EEE

[Release-log]
N/A


Change-Id: I0e7d1226098c610a639f23839cfa342ed741180d
Reviewed-on: https://gerrit.mediatek.inc/c/openwrt/feeds/mtk_openwrt_feeds/+/11801729
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