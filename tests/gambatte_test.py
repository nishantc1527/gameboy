import os
import re

import pytest

from core import check_screenshot


def _find_ref(rom):
    base = re.sub(r"\.gbc?$", "", rom)
    if os.path.exists(base + ".png"):
        return base + ".png"
    if os.path.exists(base + "_dmg08.png"):
        return base + "_dmg08.png"
    return None


all_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/gambatte/")
    for f in files
    if f.endswith(".gb") or f.endswith(".gbc")
]

roms_with_ref = [(r, _find_ref(r)) for r in all_roms if _find_ref(r)]


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom,ref", roms_with_ref)
def test_gambatte(rom, ref):
    check_screenshot(rom, ref, "gambatte")
