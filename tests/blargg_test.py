import os
from itertools import chain

import pytest

from core import check_stream

test_dirs = ["test_roms/blargg/cpu_instrs", "test_roms/blargg/instr_timing"]
roms = [
    os.path.join(d, f)
    for (d, _, files) in chain.from_iterable(os.walk(p) for p in test_dirs)
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    check_stream(rom_path, "blargg")
