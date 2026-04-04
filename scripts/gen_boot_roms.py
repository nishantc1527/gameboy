#!/usr/bin/env python3
DMG_PATH = "gb-bootroms/bin/dmg.bin"
CGB_PATH = "gb-bootroms/bin/cgb.bin"
OUT_PATH = "src/boot_roms.c"

DMG_SIZE = 0x100
CGB_SIZE = 0x900


def to_c_array(data):
    hex_bytes = [f"0x{b:02x}" for b in data]
    lines = []
    for i in range(0, len(hex_bytes), 16):
        lines.append("  " + ", ".join(hex_bytes[i : i + 16]))
    return ",\n".join(lines)


def main():
    with open(DMG_PATH, "rb") as f:
        dmg = f.read()
    with open(CGB_PATH, "rb") as f:
        cgb = f.read()

    assert len(dmg) == DMG_SIZE, (
        f"DMG boot ROM must be {DMG_SIZE} bytes, got {len(dmg)}"
    )
    assert len(cgb) == CGB_SIZE, (
        f"CGB boot ROM must be {CGB_SIZE} bytes, got {len(cgb)}"
    )

    with open(OUT_PATH, "w") as f:
        f.write("/* Auto-generated file */\n")
        f.write("#include <stdint.h>\n\n")
        f.write(
            f"const uint8_t dmg_boot_rom[{len(dmg)}] = {{\n{to_c_array(dmg)}\n}};\n\n"
        )
        f.write(
            f"const uint8_t cgb_boot_rom[{len(cgb)}] = {{\n{to_c_array(cgb)}\n}};\n"
        )

    print(f"Generated {OUT_PATH}")


if __name__ == "__main__":
    main()
