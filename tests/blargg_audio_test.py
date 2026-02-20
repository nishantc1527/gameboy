import subprocess

import pytest
from core import check_stream

from tests.core import check_stream

roms = [
    "test_roms/blargg/dmg_sound/rom_singles/01-registers.gb",
    "test_roms/blargg/dmg_sound/rom_singles/02-len ctr.gb",
]


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    check_stream(rom_path, "blargg_audio")
