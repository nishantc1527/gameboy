import pytest

from core import check_screenshot


@pytest.mark.timeout(60)
def test_rtc3_basic():
    check_screenshot(
        "test_roms/rtc3test/rtc3test.gb",
        "test_roms/rtc3test/rtc3test-basic-tests-dmg.png",
        "rtc3_basic",
    )


@pytest.mark.timeout(30)
def test_rtc3_range():
    check_screenshot(
        "test_roms/rtc3test/rtc3test.gb",
        "test_roms/rtc3test/rtc3test-range-tests-dmg.png",
        "rtc3_range",
    )


@pytest.mark.timeout(120)
def test_rtc3_subsecond():
    check_screenshot(
        "test_roms/rtc3test/rtc3test.gb",
        "test_roms/rtc3test/rtc3test-sub-second-writes-dmg.png",
        "rtc3_sub",
    )
