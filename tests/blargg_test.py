import os

import pytest

from core import check_stream

roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/cpu_instrs/individual")
    for f in files
] + ["test_roms/blargg/instr_timing/instr_timing.gb"]

# from pathlib import Path
# roms = list(Path("test_roms/blargg").rglob("*.gb"))


@pytest.mark.timeout(60)
# @pytest.mark.parametrize("rom_path", roms, ids=lambda p: p.stem)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    check_stream(rom_path, "blargg")
