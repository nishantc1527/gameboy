import subprocess

import pytest

roms = [
    "test_roms/blargg/cpu_instrs/cpu_instrs.gb",
    "test_roms/blargg/instr_timing/instr_timing.gb",
    # "test_roms/blargg/dmg_sound/dmg_sound.gb",
]

# from pathlib import Path
# roms = list(Path("test_roms/blargg").rglob("*.gb"))


@pytest.mark.timeout(60)
# @pytest.mark.parametrize("rom_path", roms, ids=lambda p: p.stem)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    cmd = ["./build/gbemu", "-r", rom_path, "--test", "blargg", "--headless"]
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    full_output = []
    status = "TIMEOUT"
    try:
        for line in iter(process.stdout.readline, ""):
            full_output.append(line)
            if "Passed" in line:
                status = "PASSED"
                process.terminate()
                break
            if "Failed" in line:
                status = "FAILED"
                process.terminate()
                break
    except Exception as e:
        process.kill()
        raise e
    finally:
        process.wait(timeout=5)
    assert status == "PASSED", f"ROM {rom_path} {status}:\n{''.join(full_output)}"
