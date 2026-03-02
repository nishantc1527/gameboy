#!/usr/bin/env python3
"""
Disassembles a rom and prints to stdout.
Most of this is just taken from the C code
in src/cpu/disassembler.c, but with some pythony
hacks to make it shorter.
"""

import sys

REGS = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]

IO_NAMES = {
    0x00: "P1",
    0x01: "SB",
    0x02: "SC",
    0x04: "DIV",
    0x05: "TIMA",
    0x06: "TMA",
    0x07: "TAC",
    0x0F: "IF",
    0x10: "NR10",
    0x11: "NR11",
    0x12: "NR12",
    0x13: "NR13",
    0x14: "NR14",
    0x16: "NR21",
    0x17: "NR22",
    0x18: "NR23",
    0x19: "NR24",
    0x1A: "NR30",
    0x1B: "NR31",
    0x1C: "NR32",
    0x1D: "NR33",
    0x1E: "NR34",
    0x20: "NR41",
    0x21: "NR42",
    0x22: "NR43",
    0x23: "NR44",
    0x24: "NR50",
    0x25: "NR51",
    0x26: "NR52",
    0x40: "LCDC",
    0x41: "STAT",
    0x42: "SCY",
    0x43: "SCX",
    0x44: "LY",
    0x45: "LYC",
    0x46: "DMA",
    0x47: "BGP",
    0x48: "OBP0",
    0x49: "OBP1",
    0x4A: "WY",
    0x4B: "WX",
    0x4D: "KEY1",
    0x4F: "VBK",
    0x50: "BOOT",
    0xFF: "IE",
}


def io_name(n):
    if n in IO_NAMES:
        return IO_NAMES[n]
    return f"${n:02X}"


def decode(rom, pc):
    if pc >= len(rom):
        return None, 0
    op = rom[pc]

    def r8():
        return rom[pc + 1] if pc + 1 < len(rom) else 0

    def r16():
        lo = rom[pc + 1] if pc + 1 < len(rom) else 0
        hi = rom[pc + 2] if pc + 2 < len(rom) else 0
        return (hi << 8) | lo

    def s8():
        v = r8()
        return v - 256 if v >= 128 else v

    def jr_abs():
        return (pc + 2 + s8()) & 0xFFFF

    if op == 0xCB:
        prfx = rom[pc + 1] if pc + 1 < len(rom) else 0
        reg = REGS[prfx & 0x07]
        bit = (prfx >> 3) & 0x07
        group = prfx >> 6
        if group == 0:
            ops = ["RLC", "RRC", "RL", "RR", "SLA", "SRA", "SWAP", "SRL"]
            return f"{ops[(prfx >> 3) & 0x07]} {reg}", 2
        elif group == 1:
            return f"BIT {bit}, {reg}", 2
        elif group == 2:
            return f"RES {bit}, {reg}", 2
        else:
            return f"SET {bit}, {reg}", 2
    match op:
        case 0x00:
            return "NOP", 1
        case 0x01:
            return f"LD BC, ${r16():04X}", 3
        case 0x02:
            return "LD (BC), A", 1
        case 0x03:
            return "INC BC", 1
        case 0x04:
            return "INC B", 1
        case 0x05:
            return "DEC B", 1
        case 0x06:
            return f"LD B, ${r8():02X}", 2
        case 0x07:
            return "RLCA", 1
        case 0x08:
            return f"LD (${r16():04X}), SP", 3
        case 0x09:
            return "ADD HL, BC", 1
        case 0x0A:
            return "LD A, (BC)", 1
        case 0x0B:
            return "DEC BC", 1
        case 0x0C:
            return "INC C", 1
        case 0x0D:
            return "DEC C", 1
        case 0x0E:
            return f"LD C, ${r8():02X}", 2
        case 0x0F:
            return "RRCA", 1
        case 0x10:
            return "STOP", 2  # STOP consumes next byte
        case 0x11:
            return f"LD DE, ${r16():04X}", 3
        case 0x12:
            return "LD (DE), A", 1
        case 0x13:
            return "INC DE", 1
        case 0x14:
            return "INC D", 1
        case 0x15:
            return "DEC D", 1
        case 0x16:
            return f"LD D, ${r8():02X}", 2
        case 0x17:
            return "RLA", 1
        case 0x18:
            return f"JR ${jr_abs():04X}", 2
        case 0x19:
            return "ADD HL, DE", 1
        case 0x1A:
            return "LD A, (DE)", 1
        case 0x1B:
            return "DEC DE", 1
        case 0x1C:
            return "INC E", 1
        case 0x1D:
            return "DEC E", 1
        case 0x1E:
            return f"LD E, ${r8():02X}", 2
        case 0x1F:
            return "RRA", 1
        case 0x20:
            return f"JR NZ, ${jr_abs():04X}", 2
        case 0x21:
            return f"LD HL, ${r16():04X}", 3
        case 0x22:
            return "LD (HL+), A", 1
        case 0x23:
            return "INC HL", 1
        case 0x24:
            return "INC H", 1
        case 0x25:
            return "DEC H", 1
        case 0x26:
            return f"LD H, ${r8():02X}", 2
        case 0x27:
            return "DAA", 1
        case 0x28:
            return f"JR Z, ${jr_abs():04X}", 2
        case 0x29:
            return "ADD HL, HL", 1
        case 0x2A:
            return "LD A, (HL+)", 1
        case 0x2B:
            return "DEC HL", 1
        case 0x2C:
            return "INC L", 1
        case 0x2D:
            return "DEC L", 1
        case 0x2E:
            return f"LD L, ${r8():02X}", 2
        case 0x2F:
            return "CPL", 1
        case 0x30:
            return f"JR NC, ${jr_abs():04X}", 2
        case 0x31:
            return f"LD SP, ${r16():04X}", 3
        case 0x32:
            return "LD (HL-), A", 1
        case 0x33:
            return "INC SP", 1
        case 0x34:
            return "INC (HL)", 1
        case 0x35:
            return "DEC (HL)", 1
        case 0x36:
            return f"LD (HL), ${r8():02X}", 2
        case 0x37:
            return "SCF", 1
        case 0x38:
            return f"JR C, ${jr_abs():04X}", 2
        case 0x39:
            return "ADD HL, SP", 1
        case 0x3A:
            return "LD A, (HL-)", 1
        case 0x3B:
            return "DEC SP", 1
        case 0x3C:
            return "INC A", 1
        case 0x3D:
            return "DEC A", 1
        case 0x3E:
            return f"LD A, ${r8():02X}", 2
        case 0x3F:
            return "CCF", 1
        case _ if 0x40 <= op <= 0x7F:
            if op == 0x76:
                return "HALT", 1
            dst = REGS[(op >> 3) & 0x07]
            src = REGS[op & 0x07]
            return f"LD {dst}, {src}", 1
        case _ if 0x80 <= op <= 0xBF:
            ops = ["ADD A,", "ADC A,", "SUB", "SBC A,", "AND", "XOR", "OR", "CP"]
            mnem = ops[(op >> 3) & 0x07]
            reg = REGS[op & 0x07]
            return f"{mnem} {reg}", 1
        case 0xC0:
            return "RET NZ", 1
        case 0xC1:
            return "POP BC", 1
        case 0xC2:
            return f"JP NZ, ${r16():04X}", 3
        case 0xC3:
            return f"JP ${r16():04X}", 3
        case 0xC4:
            return f"CALL NZ, ${r16():04X}", 3
        case 0xC5:
            return "PUSH BC", 1
        case 0xC6:
            return f"ADD A, ${r8():02X}", 2
        case 0xC7:
            return "RST $00", 1
        case 0xC8:
            return "RET Z", 1
        case 0xC9:
            return "RET", 1
        case 0xCA:
            return f"JP Z, ${r16():04X}", 3
        case 0xCC:
            return f"CALL Z, ${r16():04X}", 3
        case 0xCD:
            return f"CALL ${r16():04X}", 3
        case 0xCE:
            return f"ADC A, ${r8():02X}", 2
        case 0xCF:
            return "RST $08", 1
        case 0xD0:
            return "RET NC", 1
        case 0xD1:
            return "POP DE", 1
        case 0xD2:
            return f"JP NC, ${r16():04X}", 3
        case 0xD4:
            return f"CALL NC, ${r16():04X}", 3
        case 0xD5:
            return "PUSH DE", 1
        case 0xD6:
            return f"SUB ${r8():02X}", 2
        case 0xD7:
            return "RST $10", 1
        case 0xD8:
            return "RET C", 1
        case 0xD9:
            return "RETI", 1
        case 0xDA:
            return f"JP C, ${r16():04X}", 3
        case 0xDC:
            return f"CALL C, ${r16():04X}", 3
        case 0xDE:
            return f"SBC A, ${r8():02X}", 2
        case 0xDF:
            return "RST $18", 1
        case 0xE0:
            n = r8()
            return f"LDH ({io_name(n)}), A", 2
        case 0xE1:
            return "POP HL", 1
        case 0xE2:
            return "LDH (C), A", 1
        case 0xE5:
            return "PUSH HL", 1
        case 0xE6:
            return f"AND ${r8():02X}", 2
        case 0xE7:
            return "RST $20", 1
        case 0xE8:
            return f"ADD SP, {s8():+d}", 2
        case 0xE9:
            return "JP HL", 1
        case 0xEA:
            return f"LD (${r16():04X}), A", 3
        case 0xEE:
            return f"XOR ${r8():02X}", 2
        case 0xEF:
            return "RST $28", 1
        case 0xF0:
            n = r8()
            return f"LDH A, ({io_name(n)})", 2
        case 0xF1:
            return "POP AF", 1
        case 0xF2:
            return "LDH A, (C)", 1
        case 0xF3:
            return "DI", 1
        case 0xF5:
            return "PUSH AF", 1
        case 0xF6:
            return f"OR ${r8():02X}", 2
        case 0xF7:
            return "RST $30", 1
        case 0xF8:
            return f"LD HL, SP+{s8():+d}", 2
        case 0xF9:
            return "LD SP, HL", 1
        case 0xFA:
            return f"LD A, (${r16():04X})", 3
        case 0xFB:
            return "EI", 1
        case 0xFE:
            return f"CP ${r8():02X}", 2
        case 0xFF:
            return "RST $38", 1
        case _:
            return f"DB ${op:02X}", 1


def disassemble(rom, addr_from=0, addr_to=None):
    if addr_to is None:
        addr_to = len(rom)
    pc = 0
    while pc < len(rom):
        mnem, length = decode(rom, pc)
        if mnem is None:
            break
        if pc >= addr_from and pc < addr_to:
            bytes_str = " ".join(f"{rom[pc + i]:02X}" for i in range(length))
            print(f"${pc:04X}  {bytes_str:<8}  {mnem}")
        pc += length


def main():
    args = sys.argv[1:]
    if not args or args[0].startswith("-"):
        print(__doc__, file=sys.stderr)
        sys.exit(1)

    rom_file = args[0]
    addr_from = 0
    addr_to = None

    i = 1
    while i < len(args):
        if args[i] == "--from" and i + 1 < len(args):
            addr_from = int(args[i + 1], 16)
            i += 2
        elif args[i] == "--to" and i + 1 < len(args):
            addr_to = int(args[i + 1], 16)
            i += 2
        else:
            print(f"Unknown argument: {args[i]}", file=sys.stderr)
            sys.exit(1)

    with open(rom_file, "rb") as f:
        rom = f.read()
    disassemble(rom, addr_from=addr_from, addr_to=addr_to)


if __name__ == "__main__":
    main()
