import pytest
from core import check_screenshot


@pytest.mark.timeout(30)
@pytest.mark.skip()
def test_strikethrough():
    check_screenshot(
        "test_roms/strikethrough/strikethrough.gb",
        "test_roms/strikethrough/strikethrough-dmg.png",
        "strike",
    )
