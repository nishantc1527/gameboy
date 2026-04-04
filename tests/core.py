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


def run_with_screenshot(rom_path, ref_path, extra_args):
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as f:
        tmp = f.name
    try:
        cmd = [
            "./build/gbemu_headless",
            "-r",
            rom_path,
            "--screenshot",
            tmp,
        ] + extra_args
        subprocess.run(cmd, capture_output=True, text=True, timeout=30, check=True)
        out = Image.open(tmp).convert("RGB")
        ref = Image.open(ref_path).convert("RGB")
        out_px = list(out.getdata())
        ref_px = list(ref.getdata())
        diffs = sum(1 for a, b in zip(out_px, ref_px) if a != b)
        assert diffs == 0, (
            f"ROM {rom_path} screenshot differs from {ref_path}: "
            f"{diffs}/{len(ref_px)} pixels wrong"
        )
    finally:
        os.unlink(tmp)


def check_screenshot(rom_path, ref_path, test_category):
    run_with_screenshot(rom_path, ref_path, ["-t", test_category])


def check_screenshot_at_frame(rom_path, ref_path, frame):
    run_with_screenshot(rom_path, ref_path, ["--stop-frame", str(frame)])
