#ifndef DISASSEMBLY
#define DISASSEMBLY

#include <stdio.h>
#include <stdlib.h>

#include "var.h"
#include "func.h"

int p_instr(byte instr) {
  return 0;
  // if(!LOG) return 0;
  printf("$%04X %02X ", PC, instr);
  if(instr == 0xCB) {
    byte prfx = rd8();
    PC --;
    printf("%02X ", prfx);
    switch(prfx) {
      case 0x11:
        printf("RL C\n");
        break;
      case 0x37:
        printf("SWAP A\n");
        break;
      case 0x7C:
        printf("BIT 7, H\n");
        break;
      default:
        printf("UNKONWN PREFIX INSTRUCTION %02X\n", prfx);
        return 1;
    }
  } else {
    switch(instr) {
      case 0x00:
        printf("NOP\n");
        break;
      case 0x01:
        printf("LD BC, $%04X\n", rd16());
        PC -= 2;
        break;
      case 0x04:
        printf("INC B\n");
        break;
      case 0x05:
        printf("DEC B\n");
        break;
      case 0x06:
        printf("LD B, $%02X\n", rd8());
        PC --;
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
        printf("LD C, $%02X\n", rd8());
        PC --;
        break;
      case 0x11:
        printf("LD DE, $%04X\n", rd16());
        PC -= 2;
        break;
      case 0x13:
        printf("INC DE\n");
        break;
      case 0x15:
        printf("DEC D\n");
        break;
      case 0x16:
        printf("LD D, $%02X\n", rd8());
        PC --;
        break;
      case 0x17:
        printf("RLA\n");
        break;
      case 0x18:
        printf("JR %d\n", (signed char) rd8());
        PC --;
        break;
      case 0x19:
        printf("ADD HL, DE\n");
        break;
      case 0x1A:
        printf("LD A, (DE)\n");
        break;
      case 0x1D:
        printf("DEC E\n");
        break;
      case 0x1E:
        printf("LD E, $%02X\n", rd8());
        PC --;
        break;
      case 0x20:
        printf("JR NZ, %d\n", (signed char) rd8());
        PC --;
        break;
      case 0x21:
        printf("LD HL, $%04X\n", rd16());
        PC -= 2;
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
      case 0x28:
        printf("JR Z, %d\n", (signed char) rd8());
        PC --;
        break;
      case 0x2A:
        printf("LD A, (HL+)\n");
        break;
      case 0x2E:
        printf("LD L, $%02X\n", rd8());
        PC --;
        break;
      case 0x2F:
        printf("CPL\n");
        break;
      case 0x31:
        printf("LD SP $%04X\n", rd16());
        PC -= 2;
        break;
      case 0x32:
        printf("LD (HL-), A\n");
        break;
      case 0x36:
        printf("LD (HL), $%02X\n", rd8());
        PC --;
        break;
      case 0x3D:
        printf("DEC A\n");
        break;
      case 0x3E:
        printf("LD A, $%02X\n", rd8());
        PC --;
        break;
      case 0x47:
        printf("LD B, A\n");
        break;
      case 0x4F:
        printf("LD C, A\n");
        break;
      case 0x57:
        printf("LD D, A\n");
        break;
      case 0x5F:
        printf("LD E, A\n");
        break;
      case 0x67:
        printf("LD H, A\n");
        break;
      case 0x77:
        printf("LD (HL), A\n");
        break;
      case 0x78:
        printf("LD A, B\n");
        break;
      case 0x79:
        printf("LD A, C\n");
        break;
      case 0x7B:
        printf("LD A, E\n");
        break;
      case 0x7C:
        printf("LD A, H\n");
        break;
      case 0x7D:
        printf("LD A, L\n");
        break;
      case 0x86:
        printf("ADD A, (HL)\n");
        break;
      case 0x87:
        printf("ADD A, A\n");
        break;
      case 0x90:
        printf("SUB B\n");
        break;
      case 0xA1:
        printf("AND C\n");
        break;
      case 0xA9:
        printf("XOR C\n");
        break;
      case 0xAF:
        printf("XOR A\n");
        break;
      case 0xB0:
        printf("OR B\n");
        break;
      case 0xB1:
        printf("OR C\n");
        break;
      case 0xBE:
        printf("CP (HL)\n");
        break;
      case 0xC1:
        printf("POP BC\n");
        break;
      case 0xC3:
        printf("JP $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xC5:
        printf("PUSH BC\n");
        break;
      case 0xC9:
        printf("RET\n");
        break;
      case 0xCD:
        printf("CALL $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xE0:
        printf("LD ($FF00+%02X), A\n", rd8());
        PC --;
        break;
      case 0xE1:
        printf("POP HL\n");
        break;
      case 0xE2:
        printf("LD ($FF00+C), A\n");
        break;
      case 0xE6:
        printf("AND $%02X\n", rd8());
        PC --;
        break;
      case 0xEA:
        printf("LD ($%04X), A\n", rd16());
        PC -= 2;
        break;
      case 0xEF:
        printf("RST 5\n");
        break;
      case 0xF0:
        printf("LD A, ($FF00+$%02X)\n", rd8());
        PC --;
        break;
      case 0xF3:
        printf("DI\n");
        break;
      case 0xFB:
        printf("EI\n");
        break;
      case 0xFE:
        printf("CP $%02X\n", rd8());
        PC --;
        break;
      default:
        printf("UNKNOWN INSTRUCTION %02X\n", instr);
        return 1;
    }
  }
  return 0;
}

#endif
