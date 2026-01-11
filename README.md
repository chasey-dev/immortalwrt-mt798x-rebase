# ImmortalWrt - MT798x

```
This repository is worked on ImmortalWrt with MTK OpenWrt Feeds patches imported.
```

## Commit Cutoff Revisions

### ImmortalWrt: [21a2c44](https://github.com/immortalwrt/immortalwrt/commit/21a2c44c7a936d4eb68dd9ca3c0de99f47567f4f) - near OpenWrt 25.12.0-rc2

```
qualcommax: disable initramfs-factory generation for asus ax89x by default

The image is too big to be handled by asusuimage.

Signed-off-by: Tianling Shen <cnsztl@immortalwrt.org>
```

### MTK OpenWrt Feeds: [c28d401](https://git01.mediatek.com/plugins/gitiles/openwrt/feeds/mtk-openwrt-feeds/+/c28d401845042036048f023c2b936c5f036ad791)

```
[][kernel-6.12][common][net][bypass seq check of pptp data packet]

[Description]
Fix for PPTP sw path issues. In multi-core scenario, checking seq num
of pptp data packets causes severe packet drop. Several issues such as
low downlink throughput, unexpected disconnections under multi-pair
setup, fluctuating throughput are all related to it. Currently, we bypass
the seq check to avoid these issues.

[Release-log]
N/A

Change-Id: Ie3d8566b809d12571172ad64ba87fde18e44a2a6
Reviewed-on: https://gerrit.mediatek.inc/c/openwrt/feeds/mtk_openwrt_feeds/+/11631556
```

### l1parser: [081bb31](https://github.com/chasey-dev/l1parser/commit/081bb31211efc74594d25bfd1bb5811f3408a205)

```
feat(ucode): add get all device map support
```
