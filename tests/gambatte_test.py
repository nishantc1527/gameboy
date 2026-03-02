import os
import sys

import pytest

sys.path.insert(0, "scripts")
from core import check_stream

roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/gambatte/")
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom", roms)
def test_gambatte(rom):
    pass
