import subprocess

import pytest

from core import check_stream

roms = [
    "test_roms/blargg/cpu_instrs/cpu_instrs.gb",
    "test_roms/blargg/instr_timing/instr_timing.gb",
    # "test_roms/blargg/dmg_sound/dmg_sound.gb",
]

# from pathlib import Path
# roms = list(Path("test_roms/blargg").rglob("*.gb"))


@pytest.mark.timeout(60)
# @pytest.mark.parametrize("rom_path", roms, ids=lambda p: p.stem)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    check_stream(rom_path, "blargg")
