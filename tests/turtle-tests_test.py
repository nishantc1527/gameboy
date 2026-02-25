import sys

import pytest

from core import check_screenshot

roms = [
    (
        "test_roms/turtle-tests/window_y_trigger_wx_offscreen/window_y_trigger_wx_offscreen.gb",
        "test_roms/turtle-tests/window_y_trigger_wx_offscreen/window_y_trigger_wx_offscreen.png",
    ),
    (
        "test_roms/turtle-tests/window_y_trigger/window_y_trigger.gb",
        "test_roms/turtle-tests/window_y_trigger/window_y_trigger.png",
    ),
]


@pytest.mark.skip()
@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom, expected", roms)
def test_turtle(rom, expected):
    check_screenshot(
        rom,
        expected,
        "turtle",
    )
