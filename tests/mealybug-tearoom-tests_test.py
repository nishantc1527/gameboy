import pytest

from core import check_screenshot


@pytest.mark.skip()
@pytest.mark.timeout(30)
def test_mealybug():
    check_screenshot(
        "test_roms/mealybug-tearoom-tests/ppu/m2_win_en_toggle.gb",
        "test_roms/mealybug-tearoom-tests/ppu/m2_win_en_toggle_dmg_blob.png",
        "mealybug",
    )
