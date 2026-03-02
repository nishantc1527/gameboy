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
    around_addrs = []
    max_hits = 5
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
            around_addrs.append(args[i + 1].upper().lstrip("$"))
            i += 2
        elif a == "--max-hits" and i + 1 < len(args):
            max_hits = int(args[i + 1])
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

    around_state = {
        addr: {
            "buf": deque(maxlen=20),
            "hits": [],
            "after_remaining": 0,
            "hit_count": 0,
        }
        for addr in around_addrs
    }

    lines = []
    count = 0

    try:
        for line in iter(p.stdout.readline, ""):
            is_instr = line.startswith("$")

            if around_addrs and is_instr:
                tok = line.split()[0].upper().lstrip("$")
                for addr, st in around_state.items():
                    if tok == addr:
                        if st["hit_count"] < max_hits:
                            st["hits"].append((list(st["buf"]), line, []))
                            st["after_remaining"] = 20
                            st["hit_count"] += 1

            if around_addrs:
                for addr, st in around_state.items():
                    if st["after_remaining"] > 0 and st["hits"]:
                        st["hits"][-1][2].append(line)
                        st["after_remaining"] -= 1

            if around_addrs:
                for st in around_state.values():
                    st["buf"].append(line)

            if last_n is not None:
                lines.append(line)
            elif grep_pat:
                if grep_pat.search(line):
                    print(line, end="")
            elif not around_addrs:
                print(line, end="")

            if is_instr:
                count += 1
                if limit and count >= limit:
                    p.terminate()
                    break

            if "Passed" in line or "Failed" in line:
                p.terminate()
                break

    finally:
        p.wait(timeout=5)

    if last_n is not None:
        for line in lines[-last_n:]:
            print(line, end="")

    if around_addrs:
        for addr in around_addrs:
            st = around_state[addr]
            if not st["hits"]:
                print(f"Address ${addr} never reached.", file=sys.stderr)
            else:
                for before, hit, after in st["hits"]:
                    print(f"--- hit ${addr} ---")
                    for l in before:
                        print(l, end="")
                    print(hit, end="")
                    for l in after:
                        print(l, end="")
                if st["hit_count"] >= max_hits:
                    print(
                        f"  (${addr}: stopped after {max_hits} hits; "
                        f"use --max-hits to change)",
                        file=sys.stderr,
                    )


if __name__ == "__main__":
    main()
