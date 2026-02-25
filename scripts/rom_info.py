#!/usr/bin/env python3
"""
Print the header of a rom file.
"""

import sys

CART_TYPES = {
    0x00: "ROM ONLY",
    0x01: "MBC1",
    0x02: "MBC1+RAM",
    0x03: "MBC1+RAM+BATTERY",
    0x05: "MBC2",
    0x06: "MBC2+BATTERY",
    0x08: "ROM+RAM",
    0x09: "ROM+RAM+BATTERY",
    0x0B: "MMM01",
    0x0C: "MMM01+RAM",
    0x0D: "MMM01+RAM+BATTERY",
    0x0F: "MBC3+TIMER+BATTERY",
    0x10: "MBC3+TIMER+RAM+BATTERY",
    0x11: "MBC3",
    0x12: "MBC3+RAM",
    0x13: "MBC3+RAM+BATTERY",
    0x19: "MBC5",
    0x1A: "MBC5+RAM",
    0x1B: "MBC5+RAM+BATTERY",
    0x1C: "MBC5+RUMBLE",
    0x1D: "MBC5+RUMBLE+RAM",
    0x1E: "MBC5+RUMBLE+RAM+BATTERY",
    0x20: "MBC6",
    0x22: "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
    0xFC: "POCKET CAMERA",
    0xFD: "BANDAI TAMA5",
    0xFE: "HuC3",
    0xFF: "HuC1+RAM+BATTERY",
}
ROM_SIZES = {
    0: "32KB",
    1: "64KB",
    2: "128KB",
    3: "256KB",
    4: "512KB",
    5: "1MB",
    6: "2MB",
    7: "4MB",
    8: "8MB",
}
RAM_SIZES = {0: "None", 1: "2KB", 2: "8KB", 3: "32KB", 4: "128KB", 5: "64KB"}


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <rom.gb>", file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1], "rb") as f:
        rom = f.read()

    if len(rom) < 0x150:
        print("File too small to be a valid ROM.", file=sys.stderr)
        sys.exit(1)

    title = rom[0x134:0x144].rstrip(b"\x00").decode("ascii", errors="replace")
    cgb = rom[0x143]
    cgb_str = {0x80: "CGB compatible", 0xC0: "CGB only"}.get(cgb, "DMG only")
    cart_type = CART_TYPES.get(rom[0x147], f"Unknown (${rom[0x147]:02X})")
    rom_size = ROM_SIZES.get(rom[0x148], f"Unknown (${rom[0x148]:02X})")
    ram_size = RAM_SIZES.get(rom[0x149], f"Unknown (${rom[0x149]:02X})")
    dest = "Japan" if rom[0x14A] == 0x00 else "Worldwide"

    header_checksum = rom[0x14D]
    chk = 0
    for b in rom[0x134:0x14D]:
        chk = (chk - b - 1) & 0xFF
    chk_ok = "OK" if chk == header_checksum else f"BAD (expected ${chk:02X})"

    global_checksum = (rom[0x14E] << 8) | rom[0x14F]

    print(f"Title:            {title}")
    print(f"CGB flag:         ${cgb:02X} ({cgb_str})")
    print(f"Cartridge type:   ${rom[0x147]:02X} ({cart_type})")
    print(f"ROM size:         {rom_size} ({(len(rom) // 1024)}KB actual)")
    print(f"RAM size:         {ram_size}")
    print(f"Destination:      {dest}")
    print(f"Header checksum:  ${header_checksum:02X} ({chk_ok})")
    print(f"Global checksum:  ${global_checksum:04X}")
    print(
        f"Entry point:      ${rom[0x100]:02X} {rom[0x101]:02X} {rom[0x102]:02X} {rom[0x103]:02X}"
    )


if __name__ == "__main__":
    main()
