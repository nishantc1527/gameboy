import subprocess
import pytest

roms = [
    "test_roms/blargg/cpu_instrs/cpu_instrs.gb",
    "test_roms/blargg/instr_timing/instr_timing.gb"
]

@pytest.mark.timeout(20)
@pytest.mark.parametrize("rom_path", roms)
def test_blargg_rom(rom_path):
    cmd = ["./build/gbemu", "-r", rom_path, "--test", "blargg", "--headless"]
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
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
        pytest.fail(f"Error during execution: {e}")
    finally:
        process.wait(timeout=5)

    print("".join(full_output))
    assert status == "PASSED", f"ROM {rom_path} failed with output: {''.join(full_output)}"
