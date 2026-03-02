import pytest

from core import check_screenshot


@pytest.mark.timeout(30)
def test_cgb_acid2():
    check_screenshot(
        "test_roms/cgb-acid2/cgb-acid2.gbc",
        "test_roms/cgb-acid2/cgb-acid2.png",
        "acid2",
    )
