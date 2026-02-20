import subprocess


def check_stream(rom_path, test_category):
    cmd = ["./build/gbemu", "-r", rom_path, "--test", test_category, "--headless"]
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
                status = "Passed"
                process.terminate()
                break
            if "Failed" in line:
                status = "Failed"
                process.terminate()
                break
    except Exception as e:
        process.kill()
        raise e
    finally:
        process.wait(timeout=5)
    assert status == "Passed", f"ROM {rom_path} {status}:\n{''.join(full_output)}"


def check_out(rom_path, test_category):
    cmd = ["./build/gbemu", "-r", rom_path, "--test", test_category, "--headless"]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=15)
    assert "Passed" in result.stdout, f"ROM {rom_path} FAILED:\n{result.stdout}"
