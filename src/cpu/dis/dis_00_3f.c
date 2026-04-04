#include <stdint.h>
#include <stdio.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"

void dis_00_3f(struct Cpu* cpu, struct Bus* bus, uint8_t instr) {
  switch (instr) {
    case 0x00:
      printf("NOP\n");
      break;
    case 0x01:
      printf("LD BC, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0x02:
      printf("LD (BC), A\n");
      break;
    case 0x03:
      printf("INC BC\n");
      break;
    case 0x04:
      printf("INC B\n");
      break;
    case 0x05:
      printf("DEC B\n");
      break;
    case 0x06:
      printf("LD B, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x07:
      printf("RLCA\n");
      break;
    case 0x08:
      printf("LD ($%04X), SP\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0x09:
      printf("ADD HL, BC\n");
      break;
    case 0x0A:
      printf("LD A, (BC)\n");
      break;
    case 0x0B:
      printf("DEC BC\n");
      break;
    case 0x0C:
      printf("INC C\n");
      break;
    case 0x0D:
      printf("DEC C\n");
      break;
    case 0x0E:
      printf("LD C, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x0F:
      printf("RRCA\n");
      break;
    case 0x10:
      printf("STOP\n");
      break;
    case 0x11:
      printf("LD DE, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0x12:
      printf("LD (DE), A\n");
      break;
    case 0x13:
      printf("INC DE\n");
      break;
    case 0x14:
      printf("INC D\n");
      break;
    case 0x15:
      printf("DEC D\n");
      break;
    case 0x16:
      printf("LD D, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x17:
      printf("RLA\n");
      break;
    case 0x18: {
      int8_t off = (int8_t)bus_read(bus, (uint16_t)(cpu->PC + 1));
      printf("JR $%04X\n", (uint16_t)(cpu->PC + 2 + off));
      break;
    }
    case 0x19:
      printf("ADD HL, DE\n");
      break;
    case 0x1A:
      printf("LD A, (DE)\n");
      break;
    case 0x1B:
      printf("DEC DE\n");
      break;
    case 0x1C:
      printf("INC E\n");
      break;
    case 0x1D:
      printf("DEC E\n");
      break;
    case 0x1E:
      printf("LD E, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x1F:
      printf("RRA\n");
      break;
    case 0x20: {
      int8_t off = (int8_t)bus_read(bus, (uint16_t)(cpu->PC + 1));
      printf("JR NZ, $%04X\n", (uint16_t)(cpu->PC + 2 + off));
      break;
    }
    case 0x21:
      printf("LD HL, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0x22:
      printf("LD (HL+), A\n");
      break;
    case 0x23:
      printf("INC HL\n");
      break;
    case 0x24:
      printf("INC H\n");
      break;
    case 0x25:
      printf("DEC H\n");
      break;
    case 0x26:
      printf("LD H, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x27:
      printf("DAA\n");
      break;
    case 0x28: {
      int8_t off = (int8_t)bus_read(bus, (uint16_t)(cpu->PC + 1));
      printf("JR Z, $%04X\n", (uint16_t)(cpu->PC + 2 + off));
      break;
    }
    case 0x29:
      printf("ADD HL, HL\n");
      break;
    case 0x2A:
      printf("LD A, (HL+)\n");
      break;
    case 0x2B:
      printf("DEC HL\n");
      break;
    case 0x2C:
      printf("INC L\n");
      break;
    case 0x2D:
      printf("DEC L\n");
      break;
    case 0x2E:
      printf("LD L, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x2F:
      printf("CPL\n");
      break;
    case 0x30: {
      int8_t off = (int8_t)bus_read(bus, (uint16_t)(cpu->PC + 1));
      printf("JR NC, $%04X\n", (uint16_t)(cpu->PC + 2 + off));
      break;
    }
    case 0x31:
      printf("LD SP, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0x32:
      printf("LD (HL-), A\n");
      break;
    case 0x33:
      printf("INC SP\n");
      break;
    case 0x34:
      printf("INC (HL)\n");
      break;
    case 0x35:
      printf("DEC (HL)\n");
      break;
    case 0x36:
      printf("LD (HL), $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x37:
      printf("SCF\n");
      break;
    case 0x38: {
      int8_t off = (int8_t)bus_read(bus, (uint16_t)(cpu->PC + 1));
      printf("JR C, $%04X\n", (uint16_t)(cpu->PC + 2 + off));
      break;
    }
    case 0x39:
      printf("ADD HL, SP\n");
      break;
    case 0x3A:
      printf("LD A, (HL-)\n");
      break;
    case 0x3B:
      printf("DEC SP\n");
      break;
    case 0x3C:
      printf("INC A\n");
      break;
    case 0x3D:
      printf("DEC A\n");
      break;
    case 0x3E:
      printf("LD A, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0x3F:
      printf("CCF\n");
      break;
    default:
      printf("UNKNOWN INSTRUCTION %02X\n", instr);
      break;
  }
}
