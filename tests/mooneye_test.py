import sys

import pytest

sys.path.insert(0, "scripts")
from run_test import check_out

roms = [
    "test_roms/mooneye-test-suite/acceptance/halt_ime0_ei.gb",
    "test_roms/mooneye-test-suite/emulator-only/mbc1/bits_bank1.gb",
    "test_roms/mooneye-test-suite/emulator-only/mbc1/bits_bank2.gb",
]

# from pathlib import Path
# roms = list(Path("test_roms/mooneye-test-suite").rglob("*.gb"))


@pytest.mark.timeout(20)
# @pytest.mark.parametrize("rom_path", roms, ids=lambda p: p.stem)
@pytest.mark.parametrize("rom_path", roms)
def test_mooneye_rom(rom_path):
    check_out(rom_path, "mooneye")
