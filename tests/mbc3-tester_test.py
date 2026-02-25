import pytest

from core import check_screenshot


@pytest.mark.timeout(30)
def test_mbc3():
    check_screenshot(
        "test_roms/mbc3-tester/mbc3-tester.gb",
        "test_roms/mbc3-tester/mbc3-tester-dmg.png",
        "mbc3",
    )
