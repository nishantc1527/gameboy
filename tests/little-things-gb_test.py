import pytest

from core import check_screenshot

roms = [
    (
        "test_roms/little-things-gb/firstwhite.gb",
        "test_roms/little-things-gb/firstwhite-dmg-cgb.png",
    ),
    (
        "test_roms/little-things-gb/tellinglys.gb",
        "test_roms/little-things-gb/tellinglys-dmg.png",
    ),
]


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom, ref", roms)
@pytest.mark.skip()
def test_little_things(rom, ref):
    check_screenshot(rom, ref, "little")
