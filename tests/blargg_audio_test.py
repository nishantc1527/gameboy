import os

import pytest

from core import check_out

roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/dmg_sound/rom_singles")
    for f in files
]


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    check_out(rom_path, "blargg_audio")
