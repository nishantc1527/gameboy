import os

import pytest

from core import check_screenshot


def _find_ref(rom):
    base = rom[:-3]
    for suffix in ("_dmg_blob.png", "_dmg_b.png", "_cgb_c.png", "_cgb_d.png"):
        if os.path.exists(base + suffix):
            return base + suffix
    return None


ppu_roms = sorted(
    os.path.join("test_roms/mealybug-tearoom-tests/ppu", f)
    for f in os.listdir("test_roms/mealybug-tearoom-tests/ppu")
    if f.endswith(".gb")
)

ppu_with_ref = [(r, _find_ref(r)) for r in ppu_roms if _find_ref(r)]


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom,ref", ppu_with_ref)
@pytest.mark.skip()
def test_mealybug_ppu(rom, ref):
    check_screenshot(rom, ref, "mealybug")
