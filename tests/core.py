import os
import subprocess
import tempfile

from PIL import Image


def check_stream(rom_path, test_category):
    cmd = ["./build/gbemu_headless", "-r", rom_path, "--test", test_category]
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


def check_screenshot(rom_path, ref_path, test_category):
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as f:
        tmp = f.name
    try:
        cmd = [
            "./build/gbemu_headless",
            "-r",
            rom_path,
            "-t",
            test_category,
            "--screenshot",
            tmp,
        ]
        subprocess.run(cmd, capture_output=True, text=True, timeout=30, check=True)
        out = Image.open(tmp).convert("L")
        ref = Image.open(ref_path).convert("L")
        out_px = list(out.get_flattened_data())
        ref_px = list(ref.get_flattened_data())
        diffs = sum(1 for a, b in zip(out_px, ref_px) if a != b)
        assert diffs == 0, (
            f"ROM {rom_path} screenshot differs from {ref_path}: "
            f"{diffs}/{len(ref_px)} pixels wrong"
        )
    finally:
        os.unlink(tmp)
