# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [37013c8](https://github.com/immortalwrt/immortalwrt/commit/37013c8153ac6c9e11f4f9210d22832beb3bcb64)

```
Merge Official Source

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [a15454c](https://github.com/mediatek/mtk-openwrt-feeds/commit/a15454c888f4f4144c50e33b5feef4f247c5f78b)

```
[kernel-6.12][common][hnat][Fix debugfs-configured PPE settings being lost after a NETSYS SER]

[Description]
Fix debugfs-configured PPE settings being lost after a NETSYS SER.

[Root Cause]
A SER resets the PPE registers to hardware defaults, then
hnat_warm_init() re-programs them via hnat_hw_init(), which used
hardcoded constants and did not cover every register that debugfs can
configure. Only the settings hnat_hw_init() already read back from
hnat_priv survived, the rest reverted silently, and in some cases the
software state in hnat_priv no longer matched the hardware.

[Solution]
Latch the affected settings in hnat_priv (defaults set in
hnat_probe()) and program them from hnat_hw_init(), which is shared by
the cold and warm init paths. Add helpers for the registers
hnat_hw_init() did not cover, called from both hnat_hw_init() and the
debugfs handlers so each setting has a single write path.

[How to Verify]
Configure the settings through debugfs, dump the PPE registers,
trigger a SER, then confirm the registers still hold.

[Info to Customer]
N/A


Change-Id: I4cc44b41b1c1fdf229c0c393623ef820f06c9b9b
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