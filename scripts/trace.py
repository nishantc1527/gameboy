#!/usr/bin/env python3
"""
Trace output from emulator disassembly.
"""

import re
import subprocess
import sys
from collections import deque


def main():
    args = sys.argv[1:]
    if not args or args[0].startswith("--"):
        print(__doc__, file=sys.stderr)
        sys.exit(1)

    rom = args.pop(0)
    category = "blargg"
    grep_pat = None
    last_n = None
    limit = None
    around_addr = None
    watch_addrs = []

    i = 0
    while i < len(args):
        a = args[i]
        if a == "--test" and i + 1 < len(args):
            category = args[i + 1]
            i += 2
        elif a == "--grep" and i + 1 < len(args):
            grep_pat = re.compile(args[i + 1])
            i += 2
        elif a == "--last" and i + 1 < len(args):
            last_n = int(args[i + 1])
            i += 2
        elif a == "--limit" and i + 1 < len(args):
            limit = int(args[i + 1])
            i += 2
        elif a == "--around" and i + 1 < len(args):
            around_addr = args[i + 1].upper().lstrip("$")
            i += 2
        elif a == "--watch" and i + 1 < len(args):
            watch_addrs.append(args[i + 1].upper().lstrip("$"))
            i += 2
        else:
            print(f"Unknown option: {a}", file=sys.stderr)
            sys.exit(1)

    cmd = ["./build/gbemu_headless", "-r", rom, "--test", category, "--disassembly"]
    for addr in watch_addrs:
        cmd += ["--watch", addr]
    p = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
    )

    lines = []
    around_buf = deque(maxlen=20)  # lines before hit
    around_hits = []
    after_remaining = 0
    count = 0

    try:
        for line in iter(p.stdout.readline, ""):
            is_instr = line.startswith("$")

            if around_addr and is_instr and around_addr in line.split()[0].upper():
                around_hits.append((list(around_buf), line, []))
                after_remaining = 20

            if last_n is not None:
                lines.append(line)
            elif grep_pat:
                if grep_pat.search(line):
                    print(line, end="")
            elif around_addr:
                if after_remaining > 0 and around_hits:
                    around_hits[-1][2].append(line)
                    after_remaining -= 1
                else:
                    around_buf.append(line)
            else:
                print(line, end="")

            if is_instr:
                count += 1
                if limit and count >= limit:
                    p.terminate()
                    break

            if category == "blargg" or category == "blargg_audio":
                if "Passed" in line or "Failed" in line:
                    p.terminate()
                    break
    finally:
        p.wait(timeout=5)

    if last_n is not None:
        for line in lines[-last_n:]:
            print(line, end="")

    if around_addr:
        if not around_hits:
            print(f"Address ${around_addr} never reached.", file=sys.stderr)
        for before, hit, after in around_hits:
            print(f"--- hit ${around_addr} ---")
            for l in before:
                print(l, end="")
            print(hit, end="")
            for l in after:
                print(l, end="")


if __name__ == "__main__":
    main()
