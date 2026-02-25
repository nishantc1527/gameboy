import sys

import pytest

sys.path.insert(0, "scripts")
from screenshot_test import check_screenshot


@pytest.mark.timeout(30)
def test_dmg_acid2():
    check_screenshot(
        "test_roms/dmg-acid2/dmg-acid2.gb",
        "test_roms/dmg-acid2/dmg-acid2-dmg.png",
    )
