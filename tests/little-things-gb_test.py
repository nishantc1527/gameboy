import pytest

from core import check_screenshot


@pytest.mark.skip()
@pytest.mark.timeout(30)
def test_little_things():
    check_screenshot(
        "test_roms/little-things-gb/firstwhite.gb",
        "test_roms/little-things-gb/firstwhite-dmg-cgb.png",
        "little",
    )
