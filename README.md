# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [ce35a9e](https://github.com/immortalwrt/immortalwrt/commit/ce35a9e4ef1b87fbbc2146036ce6693e4242861a)

```
Merge Official Source

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [08196fa](https://github.com/mediatek/mtk-openwrt-feeds/commit/08196fa6e5d338ce88aa0002f781e2d24289b1f1)

```
[HIGH][openwrt-25][MAC80211][WiFi7][Fix build failure due to license header]

[Description]
Fix image build failure for MP4.3 release SDK.

[Root Cause]
Relevant release patch already includes license header, while internal
commit does not. Hence, duplicate license header is automatically
added when transforming commit to patch, causing patching conflict.

[Solution]
Avoid adding duplicate license header.

[Release-log]
N/A

[How to Verify]
N/A

[Info to Customer]
N/A

Change-Id: Ieb659d0c7763cf11ce2e63cc6bdc53cc2f674000
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