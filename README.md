# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [e04af5b](https://github.com/immortalwrt/immortalwrt/commit/e04af5bf78280429e9dc2c8602982416bd862076)

```
kernel: refresh patches

Fixes: 697d67e8a4ce ("Merge Official Source")
Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [8b882e5](https://git01.mediatek.com/plugins/gitiles/openwrt/feeds/mtk-openwrt-feeds/+/8b882e59cf7123f3138153e5db7a18873dee6f71)

```
[][kernel-6.12][common][eth][Fix the issue where the esw_cnt debug command cannot read the MIB]

[Description]
Fix the issue where the esw_cnt debug command cannot read the MIB.

[Root Cause]
Both the GDM and MT753x counters are cleared each time the kernel
executes mtk_esw_cnt_read(). However, when running the cat esw_cnt
debug command, the kernel may invoke mtk_esw_cnt_read() multiple times,
not just once. As a result, GDM and MT753x counter data may be lost
during the execution of the esw_cnt debug command.

[Solution]
We save the GDM counters in mtk_esw_cnt_open() and move the Switch
counters clear to mtk_esw_cnt_release().

[How to Verify]
N/A

[Info to Customer]
N/A


Change-Id: Idb28da45ee92f07ad64ad00206388e0ac06c9f19
Reviewed-on: https://gerrit.mediatek.inc/c/openwrt/feeds/mtk_openwrt_feeds/+/12183429
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