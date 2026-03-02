import os
from itertools import chain

import pytest

from core import check_stream

test_dirs = [
    "test_roms/mooneye-test-suite/acceptance/",
    "test_roms/mooneye-test-suite/emulator-only/",
    "test_roms/mooneye-test-suite-wilbertpol/acceptance/",
    "test_roms/mooneye-test-suite-wilbertpol/emulator-only/",
]
roms = [
    os.path.join(d, f)
    for (d, _, files) in chain.from_iterable(os.walk(p) for p in test_dirs)
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(20)
@pytest.mark.parametrize("rom_path", roms)
def test_mooneye_rom(rom_path):
    check_stream(rom_path, "mooneye")
