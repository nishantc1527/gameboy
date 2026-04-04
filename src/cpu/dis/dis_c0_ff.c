#include <stdint.h>
#include <stdio.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"

static const char* io_reg_name(uint8_t n) {
  switch (n) {
    case 0x00:
      return "P1";
    case 0x01:
      return "SB";
    case 0x02:
      return "SC";
    case 0x04:
      return "DIV";
    case 0x05:
      return "TIMA";
    case 0x06:
      return "TMA";
    case 0x07:
      return "TAC";
    case 0x0F:
      return "IF";
    case 0x10:
      return "NR10";
    case 0x11:
      return "NR11";
    case 0x12:
      return "NR12";
    case 0x13:
      return "NR13";
    case 0x14:
      return "NR14";
    case 0x16:
      return "NR21";
    case 0x17:
      return "NR22";
    case 0x18:
      return "NR23";
    case 0x19:
      return "NR24";
    case 0x1A:
      return "NR30";
    case 0x1B:
      return "NR31";
    case 0x1C:
      return "NR32";
    case 0x1D:
      return "NR33";
    case 0x1E:
      return "NR34";
    case 0x20:
      return "NR41";
    case 0x21:
      return "NR42";
    case 0x22:
      return "NR43";
    case 0x23:
      return "NR44";
    case 0x24:
      return "NR50";
    case 0x25:
      return "NR51";
    case 0x26:
      return "NR52";
    case 0x40:
      return "LCDC";
    case 0x41:
      return "STAT";
    case 0x42:
      return "SCY";
    case 0x43:
      return "SCX";
    case 0x44:
      return "LY";
    case 0x45:
      return "LYC";
    case 0x46:
      return "DMA";
    case 0x47:
      return "BGP";
    case 0x48:
      return "OBP0";
    case 0x49:
      return "OBP1";
    case 0x4A:
      return "WY";
    case 0x4B:
      return "WX";
    case 0x50:
      return "BOOT";
    case 0xFF:
      return "IE";
    default:
      return NULL;
  }
}

void dis_c0_ff(struct Cpu* cpu, struct Bus* bus, uint8_t instr) {
  switch (instr) {
    case 0xC0:
      printf("RET NZ\n");
      break;
    case 0xC1:
      printf("POP BC\n");
      break;
    case 0xC2:
      printf("JP NZ, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xC3:
      printf("JP $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xC4:
      printf("CALL NZ, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xC5:
      printf("PUSH BC\n");
      break;
    case 0xC6:
      printf("ADD A, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xC7:
      printf("RST $00\n");
      break;
    case 0xC8:
      printf("RET Z\n");
      break;
    case 0xC9:
      printf("RET\n");
      break;
    case 0xCA:
      printf("JP Z, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xCC:
      printf("CALL Z, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xCD:
      printf("CALL $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xCE:
      printf("ADC A, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xCF:
      printf("RST $08\n");
      break;
    case 0xD0:
      printf("RET NC\n");
      break;
    case 0xD1:
      printf("POP DE\n");
      break;
    case 0xD2:
      printf("JP NC, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xD4:
      printf("CALL NC, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xD5:
      printf("PUSH DE\n");
      break;
    case 0xD6:
      printf("SUB $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xD7:
      printf("RST $10\n");
      break;
    case 0xD8:
      printf("RET C\n");
      break;
    case 0xD9:
      printf("RETI\n");
      break;
    case 0xDA:
      printf("JP C, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xDC:
      printf("CALL C, $%04X\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xDE:
      printf("SBC A, $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xDF:
      printf("RST $18\n");
      break;
    case 0xE0: {
      uint8_t n = bus_read(bus, (uint16_t)(cpu->PC + 1));
      const char* name = io_reg_name(n);
      if (name)
        printf("LDH (%s), A\n", name);
      else
        printf("LDH ($FF%02X), A\n", n);
      break;
    }
    case 0xE1:
      printf("POP HL\n");
      break;
    case 0xE2:
      printf("LD ($FF00+C), A\n");
      break;
    case 0xE5:
      printf("PUSH HL\n");
      break;
    case 0xE6:
      printf("AND $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xE7:
      printf("RST $20\n");
      break;
    case 0xE8:
      printf("ADD SP, %d\n",
             (signed char)bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xE9:
      printf("JP HL\n");
      break;
    case 0xEA:
      printf("LD ($%04X), A\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xEE:
      printf("XOR $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xEF:
      printf("RST $28\n");
      break;
    case 0xF0: {
      uint8_t n = bus_read(bus, (uint16_t)(cpu->PC + 1));
      const char* name = io_reg_name(n);
      if (name)
        printf("LDH A, (%s)\n", name);
      else
        printf("LDH A, ($FF%02X)\n", n);
      break;
    }
    case 0xF1:
      printf("POP AF\n");
      break;
    case 0xF2:
      printf("LD A, ($FF00+C)\n");
      break;
    case 0xF3:
      printf("DI\n");
      break;
    case 0xF5:
      printf("PUSH AF\n");
      break;
    case 0xF6:
      printf("OR $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xF7:
      printf("RST $30\n");
      break;
    case 0xF8:
      printf("LD HL, SP+%d\n",
             (signed char)bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xF9:
      printf("LD SP, HL\n");
      break;
    case 0xFA:
      printf("LD A, ($%04X)\n",
             (uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 1)) |
                 ((uint16_t)bus_read(bus, (uint16_t)(cpu->PC + 2)) << 8));
      break;
    case 0xFB:
      printf("EI\n");
      break;
    case 0xFE:
      printf("CP $%02X\n", bus_read(bus, (uint16_t)(cpu->PC + 1)));
      break;
    case 0xFF:
      printf("RST $38\n");
      break;
    default:
      printf("UNKNOWN INSTRUCTION %02X\n", instr);
      break;
  }
}
