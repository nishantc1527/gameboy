import subprocess
import pytest

roms = [
    "test_roms/mooneye-test-suite/emulator-only/mbc1/bits_bank1.gb",
]

# from pathlib import Path
# roms = list(Path("test_roms/mooneye-test-suite").rglob("*.gb"))

@pytest.mark.timeout(20)
# @pytest.mark.parametrize("rom_path", roms, ids=lambda p: p.stem)
@pytest.mark.parametrize("rom_path", roms)
def test_mooneye_rom(rom_path):
    cmd = ["./build/gbemu", "-r", rom_path, "--test", "mooneye", "--headless"]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=15)
    assert "PASSED" in result.stdout, f"ROM {rom_path} FAILED:\n{result.stdout}"