# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [02d7fe0](https://github.com/immortalwrt/immortalwrt/commit/02d7fe0a15cebec1b515d2faef1631092d1effc1) - OpenWrt 25.12.0-rc3

```
rtl8188eu: fix build with mac80211 6.18

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
(cherry picked from commit f9de3d3f09808886ea88998a52660f98c962246b)
```

### MTK OpenWrt Feeds: [e61cd02](https://git01.mediatek.com/plugins/gitiles/openwrt/feeds/mtk-openwrt-feeds/+/e61cd0204ebe34f8936832d29c87f1d373be9ff5)

```
[][kernel-6.12][mt7981][dts][Add Wi-Fi device node]

[Description]
Add Wi-Fi device node for MT7981 Wi-Fi 6 kernel 6.12 codebase.

[Release-log]
N/A

Change-Id: Idbc4c5d0729a7edef80905a821d1b2ebba8b6c39
Reviewed-on: https://gerrit.mediatek.inc/c/openwrt/feeds/mtk_openwrt_feeds/+/11677401
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