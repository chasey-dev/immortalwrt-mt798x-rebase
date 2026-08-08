# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [3dacd2f](https://github.com/immortalwrt/immortalwrt/commit/3dacd2fb6a48c5963b1026c6a343ec7e67cbf810)

```
Merge Official Source

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [00cd776](https://git01.mediatek.com/plugins/gitiles/openwrt/feeds/mtk-openwrt-feeds/+/00cd776fc860588c1ce45d694614b065c57c986a)

```
[][kernel-6.12][common][hnat][Fix multicast forwarding issue after SER]

[Description]
Fix multicast forwarding issue on MT7988 after SER by initializing
multicast port mapping and handling multicast service. This resolves
the forwarding path exception observed in the TOPS MC path.

[Root Cause]
The issue occurred due to uninitialized multicast port mapping after
SER, leading to incorrect forwarding paths. The multicast service
handling was not properly invoked, causing forwarding exceptions.

[Solution]
Invoke hnat_mcast_ser_handle during warm initialization to reset
multicast group parameters and initialize multicast port mapping
using hnat_mcast_mcport_ppse_map_init. This ensures correct forwarding
path setup post-SER.

[How to Verify]
Test steps:
1. Trigger SER on the device.
2. Verify multicast forwarding paths are correctly set.
3. Check multicast traffic is forwarded as expected.
Expected: Multicast forwarding paths are correctly initialized and
traffic is forwarded without exceptions.

[Info to Customer]
N/A

Change-Id: I2277d9b4bd799ae61103d3017f6574f3cf48ec94
Reviewed-on: https://gerrit.mediatek.inc/c/openwrt/feeds/mtk_openwrt_feeds/+/12429042
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