#!/usr/bin/env bash
set -euo pipefail

port=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
export BE72PRO_SOURCE=${BE72PRO_SOURCE:-$(cd -- "$port/../.." && pwd)}
export BE72PRO_WORK=${BE72PRO_WORK:-$BE72PRO_SOURCE/build_dir/be72pro-wext}
export BE72PRO_VENDOR_TREE=${BE72PRO_VENDOR_TREE:-$BE72PRO_WORK/ruijie-source}
export FORCE_UNSAFE_CONFIGURE=1
export CURL_OPTIONS="${CURL_OPTIONS:---speed-time 30 --speed-limit 1024}"
mkdir -p "$BE72PRO_WORK"
cd "$BE72PRO_SOURCE"

case "${1:-}" in
    '') ;;
    --reuse-build)
        exec python3 "$port/package.py"
        ;;
    *) printf 'Usage: %s [--reuse-build]\n' "$0" >&2; exit 2 ;;
esac

if [[ ! -d "$BE72PRO_VENDOR_TREE/package/mtk/drivers/mt_wifi7/src" ]]; then
    revision=401e2ba1682314e48aabaa1af28d0e161aa58554
    archive="$BE72PRO_WORK/ruijie-$revision.tar.gz"
    if [[ ! -f "$archive" ]]; then
        curl -fLsS --retry 3 \
            "https://codeload.github.com/RuijieNetworksCommunity/MT798X-6.6-24.10/tar.gz/$revision" \
            -o "$archive.part"
        mv "$archive.part" "$archive"
    fi
    printf '%s  %s\n' df5e52602ec8abfaa5bc0a9d73ce74cdccd123e09c38a84b3fa1e5cef0fe5ad8 "$archive" |
        sha256sum -c -
    mkdir -p "$BE72PRO_VENDOR_TREE"
    tar -xzf "$archive" --strip-components=1 -C "$BE72PRO_VENDOR_TREE"
fi

while read -r feed _ revision url; do
    directory="feeds/$feed"
    if ! git -C "$directory" rev-parse --verify HEAD >/dev/null 2>&1 ||
        [[ ! -d "$directory/.git" ]]; then
        mkdir -p "$directory"
        git -C "$directory" init -q
        git -C "$directory" fetch --depth 1 "${url%%;*}" "$revision"
        git -C "$directory" checkout --detach FETCH_HEAD
    fi
    if [[ $(git -C "$directory" rev-parse HEAD) != "$revision" ]]; then
        printf 'Feed %s requires commit %s\n' "$feed" "$revision" >&2
        exit 1
    fi
done < "$port/feeds.lock"
./scripts/feeds update -i -a
./scripts/feeds install -a

profile=defconfig/low-mem-512m/mt7988-mt7992-be72pro-gsw-wext.config
if [[ -f .config ]] && ! cmp -s .config "$profile"; then
    backup=$(mktemp "$BE72PRO_WORK/config-before-wext.XXXXXX")
    cp .config "$backup"
    printf 'Previous build configuration saved to %s\n' "$backup"
fi
cp "$profile" .config
make defconfig
grep -qx 'CONFIG_TARGET_DEVICE_mediatek_filogic_DEVICE_ruijie_rg-be72-pro-mod=y' .config
grep -qx 'CONFIG_PACKAGE_luci-app-mtwifi-cfg=y' .config
make -j"${JOBS:-2}" REVISION=r0-5dae849eea

shopt -s nullglob
kernels=(build_dir/target-*/linux-mediatek_filogic/linux-6.12.116)
compilers=(staging_dir/toolchain-*/bin/aarch64-openwrt-linux-musl-gcc)
[[ ${#kernels[@]} == 1 && ${#compilers[@]} == 1 ]]
export BE72PRO_KERNEL_TREE="$PWD/${kernels[0]}"
export CROSS_COMPILE="$PWD/${compilers[0]%gcc}"
grep -qx 'CONFIG_WEXT_CORE=y' "$BE72PRO_KERNEL_TREE/.config"
grep -q $'\twireless_send_event\t' "$BE72PRO_KERNEL_TREE/Module.symvers"
bash "$port/build-driver.sh" --pci-only
python3 "$port/package.py"
