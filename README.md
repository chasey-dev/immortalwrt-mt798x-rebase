# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [761b7ff](https://github.com/immortalwrt/immortalwrt/commit/761b7ff22cbf7c13f28fc631bd70a84d295fd627)

```
Merge Official Source

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [3f66af2](https://github.com/mediatek/mtk-openwrt-feeds/commit/3f66af253becc66364f90669e58d59db19de6d95)

```
[kernel-6.12][common][ppe][Fix patch conflict issue]

[Description]
Fix patch conflict issue.

[Root Cause]
Openwrt 25.12 linux version upgrade.

[Solution]
N/A

[How to Verify]
N/A

[Info to Customer]
N/A


Change-Id: Ie6b704c027b61032e496c0c7dc51fe268900e7d3
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