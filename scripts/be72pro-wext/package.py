#!/usr/bin/env python3
"""Package the rebuilt BE72PRO WEXT firmware with the legacy PCI Wi-Fi stack."""

import copy
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tarfile
import tempfile

from fit import DEVICE, QUAD_GROUPS, SPI, mux_node, tree, verify_hashes
from legacy import add_legacy_wifi, copy_files

PORT = Path(__file__).resolve().parent
SOURCE = Path(os.environ['BE72PRO_SOURCE'])
BUILD = Path(os.environ['BE72PRO_WORK'])
VENDOR = Path(os.environ['BE72PRO_VENDOR_TREE'])
WORK = Path(tempfile.mkdtemp(prefix='sysupgrade-', dir=BUILD))
HOST_TOOLS = SOURCE / 'staging_dir/host/bin'
EXPORT = SOURCE / 'bin/targets/mediatek/filogic'
REVISION = 'r0-5dae849-be72pro-wext1-wifi-on'
KERNEL = '6.12.116'

OBSOLETE = {'kmod-mt_wifi_osal', 'kmod-mtk_wed', 'kmod-warp'}
REPLACEMENTS = ('kmod-mt_wifi7', 'kmod-mt_hwifi', 'kmod-mtk_pci',
                'kmod-connac_if', 'kmod-mt7992', 'mtwifi-cfg-ucode', 'wifi-profile')
COMMON = 'kmod-mt_wifi_cmn'


def run(*command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def apk(*args, **kwargs):
    return run(HOST_TOOLS / 'apk', *args, **kwargs)


def read_installed(root):
    packages = {}
    for record in (root / 'lib/apk/db/installed').read_text().strip().split('\n\n'):
        fields = {}
        files = []
        for line in record.splitlines():
            key, _, value = line.partition(':')
            if key == 'F':
                directory = Path(value)
            elif key == 'R':
                files.append(directory / value)
            elif key in 'PVCADTpoULm':
                fields[key] = value
        packages[fields['P']] = (fields, files)
    return packages


def copy_file(src, dest):
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.exists() or dest.is_symlink():
        dest.unlink()
    shutil.copy2(src, dest, follow_symlinks=False)


def new_version(name, wifi_on):
    if name.startswith('kmod-'):
        return f'{KERNEL}.2026.11.21~401e2ba-r1'
    if name == 'mtwifi-cfg-ucode':
        return '4~401e2ba-r3' if wifi_on else '4~401e2ba-r2'
    assert name == 'wifi-profile', name
    return '1~401e2ba-r1'


def build_apks(root, wifi_on, overlay):
    records = read_installed(root)
    ownership = {path: name for name, (_, paths) in records.items() for path in paths}
    overlays = {p.relative_to(overlay): p for p in overlay.rglob('*')
                if (p.is_file() or p.is_symlink())
                and not str(p.relative_to(overlay)).startswith(
                    ('etc/rc.d/', 'usr/share/be72pro-ram-probe/', 'lib/firmware/7988_WOCPU'))}
    extra_owners = {
        Path('usr/share/ucode/mtwifi/legacy.uc'): 'mtwifi-cfg-ucode',
        Path('etc/modules.d/60-mt_wifi_cmn'): COMMON,
        Path(f'lib/modules/{KERNEL}/mt_wifi_cmn.ko'): COMMON,
    }
    for path in overlays:
        assert extra_owners.get(path, ownership.get(path)) in (*REPLACEMENTS, COMMON), path

    artifacts = []
    with tarfile.open(root / 'lib/apk/db/scripts.tar.gz') as scripts:
        for name in (*REPLACEMENTS, COMMON):
            fields, old_paths = records.get(name, ({}, []))
            paths = set(old_paths) | {p for p, owner in extra_owners.items() if owner == name}
            stage = WORK / 'packages' / name
            stage.mkdir(parents=True)
            for path in sorted(paths):
                copy_file(overlays.get(path, root / path), stage / path)

            if name == 'mtwifi-cfg-ucode':
                detector = stage / 'lib/wifi/mtwifi.uc'
                defaults = detector.read_text()
                marker = '        "serialize": 1\n'
                assert defaults.count(marker) == 1
                disabled = 0 if wifi_on else 1
                defaults = defaults.replace(
                    marker, f'        "serialize": 1,\n        "disabled": {disabled}\n')
                if not wifi_on:
                    defaults = defaults.replace(
                        ' * The defaults intentionally create an open AP so users can connect first and\n'
                        ' * change SSID/security from LuCI or UCI later.',
                        ' * The radios start disabled until configured by the user.')
                detector.write_text(defaults)

            package_list = stage / f'lib/apk/packages/{name}.list'
            package_list.parent.mkdir(parents=True, exist_ok=True)
            package_list.write_text(''.join(
                f'/{path}\n' for path in sorted(paths - {package_list.relative_to(stage)})))
            deps = [dependency for dependency in fields.get('D', '').split()
                    if dependency not in OBSOLETE]
            if name == 'kmod-mt_wifi7':
                deps.append(COMMON)
            if name == COMMON:
                deps.append('kernel=' + records['kernel'][0]['V'])
            metadata = {
                'name': name,
                'version': new_version(name, wifi_on),
                'arch': 'aarch64_cortex-a53',
                'description': f'BE72PRO verified PCI MT7992 stack: {name}',
                'depends': ' '.join(deps),
                'provides': fields.get('p', name + '-any'),
            }
            args = []
            for key, value in metadata.items():
                args.extend(('--info', f'{key}:{value}'))
            if fields:
                prefix = f"{name}-{fields['V']}."
                for member in scripts.getmembers():
                    if member.name.startswith(prefix):
                        script_type = member.name.rsplit('.', 1)[1]
                        script = WORK / f'{name}.{script_type}'
                        script.write_bytes(scripts.extractfile(member).read())
                        args.extend(('--script', f'{script_type}:{script}'))
            target = WORK / f'{name}-{metadata["version"]}.apk'
            apk('mkpkg', *args, '--files', stage, '--output', target)
            artifacts.append(target)
    return overlays, artifacts


def prepare_root(wifi_on, revision, stock, overlay):
    root = WORK / 'rootfs'
    root.mkdir()
    run('rsync', '-aH', str(stock) + '/', str(root) + '/')
    overlays, packages = build_apks(root, wifi_on, overlay)
    world = root / 'etc/apk/world'
    world.write_text(''.join(line + '\n' for line in world.read_text().splitlines()
                             if line not in OBSOLETE))
    apk('--root', root, '--no-network', '--no-scripts', '--allow-untrusted',
        'add', *packages)
    installed = read_installed(root)
    assert not (OBSOLETE & installed.keys())
    for name in (*REPLACEMENTS, COMMON):
        assert installed[name][0]['V'] == new_version(name, wifi_on), name
        assert f"{name}><{installed[name][0]['C']}" in world.read_text().splitlines(), name
    for path, source in overlays.items():
        assert (root / path).read_bytes() == source.read_bytes(), path
    for name in ('mtk_warp', 'mtk_wed', 'mt_wifi_osal'):
        assert not (root / f'lib/modules/{KERNEL}/{name}.ko').exists()
    assert not (root / 'usr/share/be72pro-ram-probe').exists()
    assert not list((root / 'etc/rc.d').glob('*be72pro*'))
    assert 'kmod-sched-bpf' in installed
    for name in ('act_bpf', 'cls_bpf'):
        assert (root / f'lib/modules/{KERNEL}/{name}.ko').is_file()
    assert (root / 'etc/rc.d/S50uhttpd').is_symlink()
    assert (root / 'www/cgi-bin/luci').is_file()
    assert '192.168.1.1' in (root / 'bin/config_generate').read_text()
    assert not (root / 'etc/config/network').exists()
    assert 'ucidef_set_interfaces_lan_wan "lan1 lan2 lan3 lan4 eth2.2 eth1" eth2.1' in (
        root / 'etc/board.d/02_network').read_text()
    assert 'ruijie,rg-be72-pro-mod)\n\t\tCI_UBIPART="ubi"\n\t\tnand_do_upgrade "$1"' in (
        root / 'lib/upgrade/platform.sh').read_text()
    release = root / 'etc/openwrt_release'
    previous = re.search(r"^DISTRIB_REVISION='([^']+)'$", release.read_text(), re.MULTILINE)[1]
    for release in (release, root / 'etc/os-release'):
        release.write_text(release.read_text().replace(previous, revision))
    return root


def verify_first_boot(root, wifi_on):
    # Let the real target ucode generate defaults with a stub module directory.
    # Take the test files back out before serializing the filesystem.
    module = root / 'sys/module'
    module.mkdir()
    (module / 'mt_wifi').mkdir()
    config = root / 'etc/config/wireless'
    assert not config.exists()
    config.touch()
    try:
        run('chroot', root, '/lib/wifi/mtwifi.uc')
        wireless = {}
        for band, ssid in ((1, 'ImmortalWrt-2.4G'), (2, 'ImmortalWrt-5G')):
            radio = f'MT7992_1_{band}'
            iface = f'default_{radio}'
            wireless.update({
                f'{radio}.disabled': '0' if wifi_on else '1',
                f'{iface}.device': radio,
                f'{iface}.mode': 'ap',
                f'{iface}.network': 'lan',
                f'{iface}.ssid': ssid,
                f'{iface}.encryption': 'none',
            })
        output = run('chroot', root, '/sbin/uci', 'show', 'wireless',
                     capture_output=True, text=True).stdout
        settings = dict(line.split('=', 1) for line in output.splitlines())
        for key, value in wireless.items():
            assert settings[f'wireless.{key}'].strip("'") == value, key
        assert not any(key.endswith('.key') for key in settings)
    finally:
        config.unlink()
        (module / 'mt_wifi').rmdir()
        module.rmdir()


def verify_partitions(board):
    parts = SPI + '/flash@0/partitions/'
    assert 'mediatek,nmbm' in board[SPI + '/flash@0']
    assert board[mux_node(board)]['groups'] == QUAD_GROUPS
    fields = ('label', 'reg', 'read-only')
    layout = (
        ('bl2',          0x000000, 0x00100000, True),
        ('u-boot-env',   0x100000, 0x00080000, True),
        ('Factory',      0x180000, 0x00200000, True),
        ('product_info', 0x380000, 0x00080000, True),
        ('kdump',        0x400000, 0x00180000, False),
        ('FIP',          0x580000, 0x00200000, True),
        ('ubi',          0x780000, 0x0e880000, False),
    )
    old_parts = {f'{parts}partition@{offset:x}': (
        label.encode() + b'\0', struct.pack('>II', offset, size), b'' if ro else None)
        for label, offset, size, ro in layout}
    new_parts = {node: tuple(data.get(key) for key in fields) for node, data in board.items()
                 if node.startswith(parts) and '/' not in node[len(parts):]}
    assert new_parts == old_parts and len(new_parts) == 7
    assert new_parts[parts + 'partition@780000'] == (
        b'ubi\0', struct.pack('>II', 0x780000, 0x0e880000), None)



def prepare_wext_base(build):
    source = SOURCE
    original, = (source / 'bin/targets/mediatek/filogic').glob(
        '*-ruijie_rg-be72-pro-mod-squashfs-sysupgrade.bin')
    tree_path, = (source / 'build_dir').glob(
        f'target-*/linux-mediatek_filogic/linux-{KERNEL}')
    enabled = set((tree_path / '.config').read_text().splitlines())
    for symbol in ('MTK_LEGACY_WEXT', 'WIRELESS_EXT', 'WEXT_CORE', 'WEXT_PRIV'):
        assert f'CONFIG_{symbol}=y' in enabled, symbol
    fit = WORK / 'kernel'
    squashfs = WORK / 'stock-root.squashfs'
    directory = f'sysupgrade-{DEVICE}'
    with tarfile.open(original) as archive:
        for member, path in (('kernel', fit), ('root', squashfs)):
            with path.open('wb') as dest:
                shutil.copyfileobj(archive.extractfile(f'{directory}/{member}'), dest)
    nodes = tree(fit.read_bytes())
    verify_hashes(nodes, '/images/kernel-1')
    verify_hashes(nodes, '/images/fdt-1')
    assert 'ramdisk' not in nodes['/configurations/config-1']
    board = tree(nodes['/images/fdt-1']['data'])
    assert board['/soc/pcie@11300000']['wifi-reset-gpios'][-8:] == struct.pack('>II', 7, 1)
    assert board['/reserved-memory/ramoops@42ff0000']['reg'] == struct.pack(
        '>4I', 0, 0x42ff0000, 0, 0x10000)
    verify_partitions(board)
    exports = (tree_path / 'Module.symvers').read_text()
    assert '\twireless_send_event\t' in exports

    stock = WORK / 'stock-root'
    run('unsquashfs', '-processors', '2', '-no-progress', '-d', stock, squashfs)
    lib = stock / 'usr/lib/libiwinfo.so.20230701'
    symbols = run('readelf', '--use-dynamic', '-Ws', lib, capture_output=True, text=True)
    assert re.search(r'\bmtk_ops$', symbols.stdout, re.MULTILINE)
    assert (stock / 'usr/lib/rpcd/iwinfo.so').is_file()
    assert 'luci-app-mtwifi-cfg' in read_installed(stock)
    run('chroot', stock, '/usr/bin/ucode', '-e',
        "import * as iw from 'iwinfo'; exit(iw.mtk && iw.nl80211 ? 0 : 1);")
    overlay = WORK / 'wext-overlay'
    copy_files(overlay, build / 'driver-build', VENDOR)
    add_legacy_wifi(overlay, stock)
    return original, fit, stock, overlay


def package(fit, squashfs, revision, variant, original):
    export = EXPORT / 'be72pro-wext'
    name = f'be72pro-25.12-mtwifi-6.12.116-ubi-232.5{variant}-sysupgrade.bin'
    image = WORK / name
    directory = f'sysupgrade-{DEVICE}'
    files = {f'{directory}/kernel': fit, f'{directory}/root': squashfs}
    with tarfile.open(original) as old, tarfile.open(image, 'w', format=tarfile.USTAR_FORMAT) as out:
        members = old.getmembers()
        assert {entry.name for entry in members if entry.isfile()} == (
            files.keys() | {f'{directory}/CONTROL'})
        for entry in members:
            if entry.name in files:
                data = files[entry.name].read_bytes()
                entry = copy.copy(entry)
                entry.size = len(data)
                out.addfile(entry, io.BytesIO(data))
            else:
                out.addfile(entry, old.extractfile(entry) if entry.isfile() else None)
    metadata = json.loads((PORT / 'fixed-metadata.json').read_text())
    metadata['version']['revision'] = revision
    metadata_path = WORK / 'metadata.json'
    metadata_path.write_text(json.dumps(metadata) + '\n')
    fwtool = HOST_TOOLS / 'fwtool'
    run(fwtool, '-I', metadata_path, image)
    extracted_meta = WORK / 'metadata-verified.json'
    run(fwtool, '-i', extracted_meta, image)
    assert json.loads(extracted_meta.read_text()) == metadata
    assert metadata['supported_devices'] == ['ruijie,rg-be72-pro-mod']
    with tarfile.open(image) as archive, tarfile.open(original) as old:
        assert archive.getnames() == old.getnames()
        control = f'{directory}/CONTROL'
        assert archive.extractfile(control).read() == old.extractfile(control).read()
        for entry, src in files.items():
            assert archive.extractfile(entry).read() == src.read_bytes()
    export.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(image, export / name)
    (export / 'SHA256SUMS').write_text(f'{hashlib.sha256(image.read_bytes()).hexdigest()}  {name}\n')
    print(f'Verified and exported {export / name}', flush=True)


def main():
    original, fit, stock, overlay = prepare_wext_base(BUILD)
    root = prepare_root(True, REVISION, stock, overlay)
    verify_first_boot(root, wifi_on=True)
    squashfs = WORK / 'root'
    run('mksquashfs', root, squashfs, '-comp', 'xz', '-b', '262144',
        '-no-xattrs', '-noappend', '-processors', '2', '-no-progress')
    extracted = WORK / 'root-readback'
    run('unsquashfs', '-processors', '2', '-no-progress', '-d', extracted, squashfs)
    diff = run('rsync', '-acHni', '--delete', str(root) + '/', str(extracted) + '/',
               capture_output=True, text=True)
    assert not diff.stdout, diff.stdout
    package(fit, squashfs, REVISION, '-wext-wifi-on', original)


if __name__ == '__main__':
    main()
