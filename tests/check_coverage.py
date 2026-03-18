#!/usr/bin/env python3
import os
import re
import subprocess
import sys

# Mooneye subdirs outside accepted/ and emulator-only/ test trees:
_MOONEYE_EXCLUDED_PREFIXES = (
    "test_roms/mooneye-test-suite/misc/",
    "test_roms/mooneye-test-suite/madness/",
    "test_roms/mooneye-test-suite/manual-only/",
    "test_roms/mooneye-test-suite/utils/",
    "test_roms/mooneye-test-suite-wilbertpol/misc/",
    "test_roms/mooneye-test-suite-wilbertpol/madness/",
    "test_roms/mooneye-test-suite-wilbertpol/manual-only/",
    "test_roms/mooneye-test-suite-wilbertpol/logic-analysis/",
    "test_roms/mooneye-test-suite-wilbertpol/utils/",
)
_SCRIBBLTESTS_EXCLUDED = {
    "test_roms/scribbltests/fairylake/fairylake.gb",
    "test_roms/scribbltests/winpos/winpos.gb",
    "test_roms/scribbltests/statcount/statcount.gb",
}
_MEALYBUG_EXCLUDED_PREFIXES = (
    "test_roms/mealybug-tearoom-tests/dma/",
    "test_roms/mealybug-tearoom-tests/mbc/",
)
_MEALYBUG_PPU_NO_REF = {
    "test_roms/mealybug-tearoom-tests/ppu/win_without_bg.gb",
}


def _norm(path):
    return path.replace("\\", "/")


def _gambatte_has_ref(rom_path):
    base = re.sub(r"\.gbc?$", "", _norm(rom_path))
    return os.path.exists(base + ".png") or os.path.exists(base + "_dmg08.png")


def _is_known_not_tested(rom):
    n = _norm(rom)
    if n in _SCRIBBLTESTS_EXCLUDED:
        return True
    if n in _MEALYBUG_PPU_NO_REF:
        return True
    for prefix in _MOONEYE_EXCLUDED_PREFIXES:
        if n.startswith(prefix):
            return True
    for prefix in _MEALYBUG_EXCLUDED_PREFIXES:
        if n.startswith(prefix):
            return True
    return False


ROM_PAT = re.compile(r"test_roms/[^\]\"'>\n]*?\.gbc?(?!\w)")


def collect_tested_from_pytest():
    result = subprocess.run(
        [".venv/bin/python3", "-m", "pytest", "tests/", "--collect-only", "-q"],
        capture_output=True,
        text=True,
    )
    roms = set()
    for line in result.stdout.splitlines():
        for m in ROM_PAT.finditer(line):
            roms.add(os.path.normpath(m.group(0)))
    return roms


def collect_tested_from_sources():
    roms = set()
    for f in os.listdir("tests/"):
        if not f.endswith("_test.py"):
            continue
        with open(os.path.join("tests", f)) as fh:
            for line in fh:
                for m in ROM_PAT.finditer(line):
                    roms.add(os.path.normpath(m.group(0)))
    return roms


def collect_all_roms():
    roms = []
    for d, _, files in os.walk("test_roms/"):
        for f in files:
            if f.endswith(".gb") or f.endswith(".gbc"):
                roms.append(os.path.normpath(os.path.join(d, f)))
    return sorted(roms)


def main():
    tested = collect_tested_from_pytest() | collect_tested_from_sources()
    all_roms = collect_all_roms()

    known_not_tested = []
    no_ref = []
    real_uncovered = []

    for rom in all_roms:
        n = _norm(rom)
        if rom in tested or n in tested:
            continue
        if _is_known_not_tested(n):
            known_not_tested.append(rom)
        elif n.startswith("test_roms/gambatte/") and not _gambatte_has_ref(rom):
            no_ref.append(rom)
        else:
            real_uncovered.append(rom)

    covered = len(all_roms) - len(known_not_tested) - len(no_ref) - len(real_uncovered)
    print(f"Total ROMs on disk       : {len(all_roms)}")
    print(f"ROMs with a test         : {covered}")
    print(
        f"Known not tested         : {len(known_not_tested)} (combined runners, demos, no-ref mealybug)"
    )
    print(f"Gambatte (no ref)        : {len(no_ref)} (intentionally skipped)")
    print(f"Truly uncovered          : {len(real_uncovered)}")

    if real_uncovered:
        print()
        for rom in real_uncovered:
            print(f"  UNCOVERED  {rom}")
        sys.exit(1)
    else:
        print("All testable ROMs covered.")


if __name__ == "__main__":
    main()
