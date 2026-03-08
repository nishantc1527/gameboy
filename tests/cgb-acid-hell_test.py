import pytest

from core import check_screenshot


@pytest.mark.timeout(30)
@pytest.mark.skip
def test_cgb_acid_hell():
    check_screenshot(
        "test_roms/cgb-acid-hell/cgb-acid-hell.gbc",
        "test_roms/cgb-acid-hell/cgb-acid-hell.png",
        "acid2",
    )
