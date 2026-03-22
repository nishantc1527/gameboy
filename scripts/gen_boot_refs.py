#!/usr/bin/env python3
import os
import sys
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "tests"))
from visual_roms import BOOT_ROMS, ROMS


def _stem(rom):
    return os.path.splitext(os.path.basename(rom))[0]


def _all_tasks():
    for rom, boot, start, end in BOOT_ROMS + ROMS:
        stem = _stem(rom)
        for frame in range(start, end + 1):
            yield rom, boot, frame, f"tests/refs/{stem}/frame_{frame:03d}.png"


def _list():
    for _, _, _, ref in _all_tasks():
        print(ref)


def _generate():
    tasks = list(_all_tasks())
    total = len(tasks)

    for rom, boot, _, _ in BOOT_ROMS + ROMS:
        os.makedirs(f"tests/refs/{_stem(rom)}", exist_ok=True)

    completed = 0

    def run(task):
        rom, boot, frame, ref = task
        cmd = ["./build/gbemu_headless", "-r", rom, "--stop-frame", str(frame), "--screenshot", ref]
        if boot:
            cmd += ["-b", boot]
        subprocess.run(cmd, check=True, capture_output=True)

    with ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
        futures = {pool.submit(run, t): t for t in tasks}
        for f in as_completed(futures):
            f.result()
            completed += 1
            print(f"\r{completed}/{total}", end="", flush=True)

    print()


if __name__ == "__main__":
    if "--list" in sys.argv:
        _list()
    else:
        _generate()
