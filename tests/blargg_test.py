import os
import sys
from itertools import chain

import pytest

sys.path.insert(0, "scripts")
from run_test import check_stream
from screenshot_test import check_screenshot

test_dirs = [
    "test_roms/blargg/cpu_instrs/",
    "test_roms/blargg/instr_timing/",
    "test_roms/blargg/mem_timing/",
]
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
