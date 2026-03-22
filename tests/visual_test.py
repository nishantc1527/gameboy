import os

import pytest
from core import check_screenshot_at_frame
from visual_roms import BOOT_ROMS, ROMS


def _stem(rom):
    return os.path.splitext(os.path.basename(rom))[0]


params = [
    (rom, boot, frame, f"tests/refs/{_stem(rom)}/frame_{frame:03d}.png")
    for rom, boot, start, end in BOOT_ROMS + ROMS
    if os.path.exists(rom)
    for frame in range(start, end + 1)
]


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom,boot,frame,ref", params)
def test_visual(rom, boot, frame, ref):
    check_screenshot_at_frame(rom, ref, frame, boot_rom=boot)
