import pytest

from core import check_screenshot

roms = [
    (
        "test_roms/scribbltests/lycscx/lycscx.gb",
        "test_roms/scribbltests/lycscx/lycscx-cgb-dmg.png",
    ),
    (
        "test_roms/scribbltests/lycscy/lycscy.gb",
        "test_roms/scribbltests/lycscy/lycscy-cgb-dmg.png",
    ),
    (
        "test_roms/scribbltests/palettely/palettely.gb",
        "test_roms/scribbltests/palettely/palettely-dmg.png",
    ),
    (
        "test_roms/scribbltests/scxly/scxly.gb",
        "test_roms/scribbltests/scxly/scxly-dmg.png",
    ),
    (
        "test_roms/scribbltests/statcount/statcount-auto.gb",
        "test_roms/scribbltests/statcount/statcount_auto-cgb-dmg.png",
    ),
]


@pytest.mark.skip()
@pytest.mark.parametrize("rom, expected", roms)
@pytest.mark.timeout(30)
def test_scribble(rom, expected):
    check_screenshot(
        rom,
        expected,
        "scribble",
    )
