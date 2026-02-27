import os

import pytest

from core import check_stream

roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/gbmicrotest")
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.skip()
@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", roms)
def test_micro(rom_path):
    check_stream(rom_path, "micro")
