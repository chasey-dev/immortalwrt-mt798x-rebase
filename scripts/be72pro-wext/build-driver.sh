#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

port=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build=${BE72PRO_WORK:?}
kernel=${BE72PRO_KERNEL_TREE:?}
vendor=${BE72PRO_VENDOR_TREE:?}/package/mtk/drivers
modules=$build/driver-build
hnat=''
old_config=$port/legacy.config
patch_dir=${BE72PRO_SOURCE:?}/scripts/patch-kernel.sh
mkdir -p "$modules"
exec 8>"$modules/.build.lock"
flock 8

# These drivers use native AArch64 Kbuild and the same options as the
# working 6.6.162 image. A production image also needs runtime validation.
for package in mt_wifi_cmn warp mt_wifi7 mt_hwifi; do
    output="$modules/$package"
    if [[ ! -f "$output/.prepared" ]]; then
        mkdir -p "$output"
        cp -a "$vendor/$package/src/." "$output/"
        if [[ -d "$vendor/$package/patches" ]]; then
            "$patch_dir" "$output" "$vendor/$package/patches" > "$modules/$package-prepare.log" 2>&1
        fi
        touch "$output/.prepared"
    fi
done

if [[ ! -f "$modules/.linux-6.12-compat" ]]; then
    patch --directory "$modules" --strip 1 < "$port/ruijie-wifi-linux-6.12.patch"
    touch "$modules/.linux-6.12-compat"
fi
if [[ ! -f "$modules/.warp-6.12-compat" ]]; then
    patch --directory "$modules" --strip 1 < "$port/ruijie-warp-linux-6.12.patch"
    touch "$modules/.warp-6.12-compat"
fi
if [[ ! -f "$modules/.mtwifi-6.12-compat" ]]; then
    patch --directory "$modules" --strip 1 < "$port/ruijie-mtwifi-linux-6.12.patch"
    touch "$modules/.mtwifi-6.12-compat"
fi
if [[ ! -f "$modules/.mtwifi-mat-compat" ]]; then
    patch --directory "$modules" --strip 1 < "$port/ruijie-mtwifi-mat.patch"
    touch "$modules/.mtwifi-mat-compat"
fi
for header in "$modules"/{mt_wifi7,mt_hwifi}/mt_wifi/include/os/rt_linux.h; do
    if grep -Fq '<asm/unaligned.h>' "$header"; then
        sed -i 's@<asm/unaligned.h>@<linux/unaligned.h>@' "$header"
    fi
done
# These two functions use a vendor-local macro that actually returns the larger value.
for file in "$modules"/mt_wifi7/mt_wifi/feature/rrm/{rrm,rrm_tlv}.c; do
    if grep -qw MIN "$file"; then
        sed -i 's/\<MIN\>/RRM_MAX/g' "$file"
    fi
done
# The assignment only changed the local pointer and had no caller-visible effect.
wifi_linux="$modules/mt_wifi7/mt_wifi/os/linux"
if grep -q 'phy_addr = NULL;' "$wifi_linux/rt_rbus_pci_util.c"; then
    sed -i '/^[[:blank:]]*phy_addr = NULL;$/d' "$wifi_linux/rt_rbus_pci_util.c"
fi
cp -u "$port/ruijie-wext-compat.h" "$wifi_linux/"
if ! grep -q '"ruijie-wext-compat.h"' "$wifi_linux/sta_ioctl.c"; then
    sed -i '/^#include "rt_os_net.h"/a #include "ruijie-wext-compat.h"' "$wifi_linux/sta_ioctl.c"
    sed -i -E 's/\<iwe_stream_add_(event|point)\>\(_A/ruijie_iwe_stream_add_\1(_A/' "$wifi_linux/sta_ioctl.c"
fi
# Preserve the vendor implementations under private names where 6.12 now
# provides macros with the same identifiers.
while IFS= read -r -d '' file; do
    sed -i -E 's/\<(__bf_shf|field_prep|field_get)\>/hwifi_\1/g' "$file"
done < <(grep -rlZE '\b(__bf_shf|field_prep|field_get)\b' \
    --include='*.c' --include='*.h' "$modules/mt_hwifi/wlan_hwifi")
while IFS= read -r -d '' file; do
    sed -i 's/\<PCI_IRQ_LEGACY\>/PCI_IRQ_INTX/g' "$file"
done < <(grep -rlZw PCI_IRQ_LEGACY --include='*.c' --include='*.h' \
    "$modules/mt_hwifi/wlan_hwifi/chips/mt7992")

mapfile -t wifi_flags < <(python3 - "$old_config" "$vendor/mt_wifi7/Makefile" <<'PY'
import re
import sys
from pathlib import Path

config = dict(line.split('=', 1) for line in Path(sys.argv[1]).read_text().splitlines()
              if line.startswith('CONFIG_MTK_WIFI7_') and '=' in line)
recipe = Path(sys.argv[2]).read_text()
keys = re.search(r'^PKG_KCONFIG\s*:=((?:[^\n]*\\\n)*[^\n]*)', recipe, re.M)[1]
for key in keys.replace('\\\n', ' ').split():
    if (option := 'CONFIG_MTK_WIFI7_' + key) in config:
        print('CONFIG_' + key + '=' + config[option].strip('"'))
PY
)
[[ "${#wifi_flags[@]}" -gt 50 ]]

compile() {
    local name=$1
    local source=$2
    shift 2
    printf 'Compiling %s\n' "$name"
    make -j2 -C "$kernel" ARCH=arm64 M="$source" "$@" modules \
        > "$modules/$name.log" 2>&1 || { tail -80 "$modules/$name.log"; return 1; }
}

compile mt_wifi_cmn "$modules/mt_wifi_cmn" NOSTDINC_FLAGS=-DMT_WIFI_COMMON_SUPPORT
wifi_symbols="$modules/mt_wifi_cmn/Module.symvers $hnat"
if [[ "${1:-}" != --pci-only ]]; then
    compile warp "$modules/warp" CONFIG_SUPPORT_OPENWRT=y CONFIG_WARP_CHIPSET=mt7988 \
        CONFIG_WARP_VERSION=3 CONFIG_WARP_DBG_SUPPORT=y CONFIG_WED_HW_RRO_SUPPORT=y \
        CONFIG_MTK_MEMORY_SHRINK=y CONFIG_TARGET_mediatek=y \
        KBUILD_EXTRA_SYMBOLS="$hnat" \
        'KCFLAGS=-include linux/vmalloc.h -Wno-error=missing-prototypes -Wno-error=old-style-declaration'
    wifi_symbols+=" $modules/warp/Module.symvers"
fi
compile mt_wifi7 "$modules/mt_wifi7/mt_wifi_ap" "${wifi_flags[@]}" \
    LINUX_DIR="$build" NOSTDINC_FLAGS=-DMT_WIFI_COMMON_SUPPORT \
    "KCFLAGS=-include $port/ruijie-mtwifi-compat.h -Wno-error=missing-prototypes -Wno-error=old-style-declaration -Wno-error=empty-body" \
    KBUILD_EXTRA_SYMBOLS="$wifi_symbols"

hwifi=mt_hwifi
wed=m
hwifi_symbols="$modules/mt_wifi_cmn/Module.symvers $modules/mt_wifi7/mt_wifi_ap/Module.symvers"
if [[ "${1:-}" == --pci-only ]]; then
    hwifi=mt_hwifi_pci
    wed=n
    mkdir -p "$modules/$hwifi"
    rsync -a --exclude='*.o' --exclude='*.ko' --exclude='*.mod*' \
        --exclude='.*.cmd' --exclude='Module.symvers' --exclude='modules.order' \
        "$modules/mt_hwifi/" "$modules/$hwifi/"
else
    hwifi_symbols+=" $modules/warp/Module.symvers"
fi
compile "$hwifi" "$modules/$hwifi/wlan_hwifi" CONFIG_SUPPORT_OPENWRT=y \
    CONFIG_HWIFI_HIF_WED="$wed" CONFIG_HWIFI_HIF_NPU=n CONFIG_HWIFI_MEM_SHRINK=y \
    CONFIG_HWIFI_CHIP_MT7915=n CONFIG_HWIFI_CHIP_MT7916=n CONFIG_HWIFI_CHIP_BELLWETHER=n \
    CONFIG_HWIFI_CHIP_MT7990=n CONFIG_HWIFI_CHIP_MT7991=n CONFIG_HWIFI_CHIP_MT799A=n \
    CONFIG_HWIFI_CHIP_MT7993=n CONFIG_HWIFI_CHIP_MT799B=n CONFIG_HWIFI_CHIP_BLACKHAWK_A0=n \
    CONFIG_HWIFI_CHIP_MT7915_DBG=n CONFIG_HWIFI_CHIP_MT7990_DBG=n CONFIG_HWIFI_CHIP_MT7992_DBG=n \
    CONFIG_HWIFI_CHIP_MT7993_DBG=n \
    "KCFLAGS=-DMT_WIFI_COMMON_SUPPORT -include $port/ruijie-mtwifi-compat.h -Wno-error=missing-prototypes -Wno-error=old-style-declaration" \
    MTK_WED="$modules/warp" LINUX_DIR="$build" \
    KBUILD_EXTRA_SYMBOLS="$hwifi_symbols"
find "$modules/$hwifi" -name '*.ko' -type f | sort
