import sys

import pytest

from core import check_screenshot


@pytest.mark.skip()
@pytest.mark.timeout(30)
def test_strikethrough():
    check_screenshot(
        "test_roms/strikethrough/strikethrough.gb",
        "test_roms/strikethrough/strikethrough-dmg.png",
        "strike",
    )
