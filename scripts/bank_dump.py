#!/usr/bin/env python3
import argparse

BANK_SIZE = 0x4000


def parse_hex(s):
    return int(s, 16)


def num_banks(rom):
    rom_size_byte = rom[0x148]
    return 1 << (rom_size_byte + 1)


def main():
    parser = argparse.ArgumentParser(
        description="Dump ROM bank contents for debugging bank switching."
    )
    parser.add_argument("rom", help="Path to the .gb ROM file")
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        "--offset",
        type=parse_hex,
        default=0,
        metavar="HEX",
        help="Offset within each bank to dump (hex, default: 0)",
    )
    group.add_argument(
        "--addr",
        type=parse_hex,
        metavar="HEX",
        help="GB address to dump per bank (e.g. 4000 for the banked window start)",
    )
    group.add_argument(
        "--raw",
        nargs=2,
        metavar=("START", "END"),
        type=parse_hex,
        help="Dump raw ROM file bytes from START to END (hex addresses, inclusive)",
    )
    parser.add_argument(
        "--count",
        type=parse_hex,
        default=1,
        metavar="HEX",
        help="Number of bytes to show per bank (hex, default: 1)",
    )
    args = parser.parse_args()

    with open(args.rom, "rb") as f:
        rom = bytearray(f.read())

    n_banks = num_banks(rom)
    rom_size_byte = rom[0x148]
    print(f"ROM: {args.rom}")
    print(
        f"ROM size byte: 0x{rom_size_byte:02X}  ({n_banks} banks x 16KB = {n_banks * 16}KB)"
    )
    print()

    if args.raw is not None:
        start, end = args.raw
        data = rom[start : end + 1]
        print(f"Raw bytes ${start:04X}–${end:04X}:")
        for i, b in enumerate(data):
            print(f"  ${start + i:04X}: 0x{b:02X}  ({b})")
        return

    if args.addr is not None:
        if args.addr < 0x4000:
            offset = args.addr
            data = rom[offset : offset + args.count]
            print(f"Fixed bank 0, GB addr ${args.addr:04X}:")
            print(f"  {' '.join(f'0x{b:02X}' for b in data)}")
            return
        else:
            offset = args.addr - 0x4000
    else:
        offset = args.offset

    count = args.count
    print(f"Bank offset 0x{offset:04X}, {count} byte(s) per bank:")
    print()
    for bank in range(n_banks):
        file_offset = bank * BANK_SIZE + offset
        if file_offset + count > len(rom):
            data = rom[file_offset:] + bytes(count - (len(rom) - file_offset))
        else:
            data = rom[file_offset : file_offset + count]
        hex_bytes = " ".join(f"0x{b:02X}" for b in data)
        ascii_repr = "".join(chr(b) if 0x20 <= b < 0x7F else "." for b in data)
        print(
            f"  Bank {bank:2d} (file ${file_offset:05X}): {hex_bytes}  |{ascii_repr}|"
        )


if __name__ == "__main__":
    main()
