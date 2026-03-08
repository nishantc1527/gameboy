import os
import sys

import pytest

from core import check_stream

roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/age-test-roms")
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom", roms)
@pytest.mark.skip()
def test_age(rom):
    check_stream(rom, "age")
