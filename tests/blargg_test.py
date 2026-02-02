import subprocess

import pytest

BLARGG_FILES = [
    "test_roms/blargg/cpu_instrs/cpu_instrs.gb",
    "test_roms/blargg/instr_timing/instr_timing.gb",
]

@pytest.mark.parametrize("rom", BLARGG_FILES)
def test_blargg(rom):
    res = subprocess.run(
        ["./build/gbemu", "-r", rom, "-t", "blargg", "-h"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=120,
    )
    output = res.stdout + res.stderr
    assert res.returncode == 0, f"Emulator crashed on {rom}\n{output}"
    assert "Passed" in output, f"Blargg failed on {rom}\n{output}"
