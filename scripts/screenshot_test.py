#!/usr/bin/env python3
"""
Compare emulator screenshot to correct screenshot.
"""

import os
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Pillow not installed. Run: pip install pillow", file=sys.stderr)
    sys.exit(2)


def check_screenshot(rom_path, ref_path, test_category="acid2"):
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


def run_screenshot(rom, out_path):
    cmd = ["./build/gbemu_headless", "-r", rom, "-t", "acid2", "--screenshot", out_path]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        return False
    return True


def compare(out_path, ref_path, save_diff=None):
    out = Image.open(out_path).convert("L")
    ref = Image.open(ref_path).convert("L")

    if out.size != ref.size:
        print(f"Size mismatch: got {out.size}, expected {ref.size}")
        return False

    out_px = list(out.get_flattened_data())
    ref_px = list(ref.get_flattened_data())
    w, h = ref.size

    diffs = [
        (i % w, i // w, ref_px[i], out_px[i])
        for i in range(len(ref_px))
        if ref_px[i] != out_px[i]
    ]

    total = w * h
    if not diffs:
        print(f"PASSED — images match ({total} pixels)")
        return True

    print(
        f"FAILED — {len(diffs)}/{total} pixels differ ({100 * len(diffs) / total:.1f}%)"
    )

    print("\nFirst differing pixels (x, y, expected, got):")
    for x, y, exp, got in diffs[:20]:
        print(f"  ({x:3d},{y:3d})  expected=${exp:02X}  got=${got:02X}")
    if len(diffs) > 20:
        print(f"  ... and {len(diffs) - 20} more")

    from collections import Counter

    row_counts = Counter(y for _, y, _, _ in diffs)
    print("\nWorst rows:")
    for row, count in row_counts.most_common(5):
        print(f"  row {row:3d}: {count} px wrong")

    if save_diff:
        diff_img = out.convert("RGB")
        pixels = diff_img.load()
        for x, y, _, _ in diffs:
            pixels[x, y] = (255, 0, 0)
        diff_img.save(save_diff)
        print(f"\nDiff image saved to: {save_diff}")

    return False


def main():
    args = sys.argv[1:]
    if len(args) < 2 or args[0].startswith("--"):
        print(__doc__, file=sys.stderr)
        sys.exit(1)

    rom = args[0]
    ref = args[1]
    save = None

    i = 2
    while i < len(args):
        if args[i] == "--save" and i + 1 < len(args):
            save = args[i + 1]
            i += 2
        else:
            print(f"Unknown option: {args[i]}", file=sys.stderr)
            sys.exit(1)

    if not Path(rom).exists():
        print(f"ROM not found: {rom}", file=sys.stderr)
        sys.exit(1)
    if not Path(ref).exists():
        print(f"Reference not found: {ref}", file=sys.stderr)
        sys.exit(1)

    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as f:
        tmp = f.name
    try:
        if not run_screenshot(rom, tmp):
            sys.exit(1)
        ok = compare(tmp, ref, save_diff=save)
        sys.exit(0 if ok else 1)
    finally:
        os.unlink(tmp)


if __name__ == "__main__":
    main()
