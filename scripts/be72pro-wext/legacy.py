"""Stage the six legacy Wi-Fi modules and their board-specific inputs."""

import hashlib
import json
from pathlib import Path
import shutil
import subprocess


PORT = Path(__file__).resolve().parent
VERSION = '6.12.116'
MODULES = (
    'mt_wifi_cmn/mt_wifi_cmn.ko',
    'mt_wifi7/mt_wifi_ap/mt_wifi.ko',
    'mt_hwifi_pci/wlan_hwifi/mtk_hwifi.ko',
    'mt_hwifi_pci/wlan_hwifi/mtk_pci.ko',
    'mt_hwifi_pci/wlan_hwifi/connac_if.ko',
    'mt_hwifi_pci/wlan_hwifi/mt7992.ko',
)


def copy_files(overlay, driver_tree, vendor):
    shutil.copytree(PORT / 'legacy-root', overlay, symlinks=True)
    modules = overlay / 'lib/modules' / VERSION
    modules.mkdir(parents=True)
    for relative in MODULES:
        src = driver_tree / relative
        modinfo = subprocess.check_output(['readelf', '-p', '.modinfo', src], text=True)
        assert f'vermagic={VERSION} SMP mod_unload aarch64' in modinfo, src
        assert 'mtk_warp' not in modinfo and 'mtk_wed' not in modinfo, src
        sections = subprocess.check_output(['readelf', '-SW', src], text=True)
        assert '__versions' not in sections, src
        subprocess.run(['objcopy', '--strip-debug', src, modules / src.name], check=True)

    firmware = overlay / 'lib/firmware'
    firmware.mkdir()
    for name, item in json.loads((PORT / 'firmware-map.json').read_text()).items():
        source = vendor / item['source']
        assert hashlib.sha256(source.read_bytes()).hexdigest() == item['sha256'], source
        shutil.copy2(source, firmware / name)

    autoload = overlay / 'etc/modules.d/66-mt7992'
    params = autoload.read_text()
    assert params.strip() == 'mt7992 option_type=0 rro_mode=4 intr_option_set=0'
    autoload.write_text(params.replace('rro_mode=4', 'rro_mode=0'))
    shared_profile = overlay / 'etc/wireless/mediatek/mt7992.1.dat'
    settings = shared_profile.read_text()
    assert settings.splitlines().count('WHNAT=1') == 1
    shared_profile.write_text(settings.replace('WHNAT=1', 'WHNAT=0'))


def add_legacy_wifi(overlay, stock):
    for relative, source, mode in (
        ('lib/netifd/wireless/mtwifi.sh', stock / 'lib/netifd/wireless/mtwifi.sh', 0o755),
        ('usr/share/ucode/mtwifi/legacy.uc', PORT / 'ruijie-legacy-mtwifi.uc', 0o644),
    ):
        target = overlay / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)
        target.chmod(mode)
    subprocess.run(['patch', '-d', overlay, '-p1', '-i', PORT / 'ruijie-legacy-mtwifi.patch'],
                   check=True)
