import os

import pytest
from core import check_screenshot_at_frame
from visual_roms import ROMS


def _stem(rom):
    return os.path.splitext(os.path.basename(rom))[0]


params = [
    (rom, frame, f"tests/refs/{_stem(rom)}/frame_{frame:03d}.png")
    for rom, start, end in ROMS
    for frame in range(start, end + 1)
]


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom,frame,ref", params)
def test_visual(rom, frame, ref):
    check_screenshot_at_frame(rom, ref, frame)
