#ifndef DISASSEMBLY
#define DISASSEMBLY

#include <stdio.h>
#include <stdlib.h>

#include "var.h"
#include "func.h"

int p_instr(byte instr) {
  return 0;
  if(!r_mem(0xFF50)) return 0;
  printf("$%04X %02X ", PC, instr);
  if(instr == 0xCB) {
    byte prfx = rd8();
    PC --;
    printf("%02X ", prfx);
    switch(prfx) {
      case 0x11:
        printf("RL C\n");
        break;
      case 0x19:
        printf("RR C\n");
        break;
      case 0x1A:
        printf("RR D\n");
        break;
      case 0x37:
        printf("SWAP A\n");
        break;
      case 0x38:
        printf("SRL B\n");
        break;
      case 0x7C:
        printf("BIT 7, H\n");
        break;
      case 0x7E:
        printf("BIT 7, (HL)\n");
        break;
      case 0x86:
        printf("RES 0, (HL)\n");
        break;
      case 0x87:
        printf("RES 0, A\n");
        break;
      case 0x8E:
        printf("RES 1, (HL)\n");
        break;
      case 0x96:
        printf("RES 2, (HL)\n");
        break;
      case 0x9E:
        printf("RES 3, (HL)\n");
        break;
      case 0xA6:
        printf("RES 4, (HL)\n");
        break;
      case 0xAE:
        printf("RES 5, (HL)\n");
        break;
      case 0xB6:
        printf("RES 6, (HL)\n");
        break;
      case 0xC6:
        printf("SET 0, (HL)\n");
        break;
      case 0xCE:
        printf("SET 1, (HL)\n");
        break;
      case 0xD6:
        printf("SET 2, (HL)\n");
        break;
      case 0xDE:
        printf("SET 3, (HL)\n");
        break;
      case 0xE6:
        printf("SET 4, (HL)\n");
        break;
      case 0xEE:
        printf("SET 5, (HL)\n");
        break;
      case 0xF6:
        printf("SET 6, (HL)\n");
        break;
      case 0xFE:
        printf("SET 7, (HL)\n");
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
        printf("LD B, $%02X\n", rd8());
        PC --;
        break;
      case 0x07:
        printf("RLCA\n");
        break;
      case 0x08:
        printf("LD ($%04X), SP\n", rd16());
        PC -= 2;
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
        printf("LD C, $%02X\n", rd8());
        PC --;
        break;
      case 0x0F:
        printf("RRCA\n");
        break;
      case 0x10:
        printf("STOP\n");
        break;
      case 0x11:
        printf("LD DE, $%04X\n", rd16());
        PC -= 2;
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
        printf("LD E, $%02X\n", rd8());
        PC --;
        break;
      case 0x1F:
        printf("RRA\n");
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
      case 0x25:
        printf("DEC H\n");
        break;
      case 0x26:
        printf("LD H, $%02X\n", rd8());
        PC --;
        break;
      case 0x27:
        printf("DAA\n");
        break;
      case 0x28:
        printf("JR Z, %d\n", (signed char) rd8());
        PC --;
        break;
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
        printf("LD L, $%02X\n", rd8());
        PC --;
        break;
      case 0x2F:
        printf("CPL\n");
        break;
      case 0x30:
        printf("JR NC, %d\n", (signed char) rd8());
        PC --;
        break;
      case 0x31:
        printf("LD SP $%04X\n", rd16());
        PC -= 2;
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
        printf("LD (HL), $%02X\n", rd8());
        PC --;
        break;
      case 0x37:
        printf("SCF\n");
        break;
      case 0x38:
        printf("JR C, %d\n", (signed char) rd8());
        PC --;
        break;
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
        printf("LD A, $%02X\n", rd8());
        PC --;
        break;
      case 0x3F:
        printf("CCF\n");
        break;
      case 0x40:
        printf("LD B, B\n");
        break;
      case 0x41:
        printf("LD B, C\n");
        break;
      case 0x42:
        printf("LD B, D\n");
        break;
      case 0x43:
        printf("LD B, E\n");
        break;
      case 0x44:
        printf("LD B, H\n");
        break;
      case 0x45:
        printf("LD B, L\n");
        break;
      case 0x46:
        printf("LD B, (HL)\n");
        break;
      case 0x47:
        printf("LD B, A\n");
        break;
      case 0x48:
        printf("LD C, B\n");
        break;
      case 0x49:
        printf("LD C, C\n");
        break;
      case 0x4A:
        printf("LD C, D\n");
        break;
      case 0x4B:
        printf("LD C, E\n");
        break;
      case 0x4C:
        printf("LD C, H\n");
        break;
      case 0x4D:
        printf("LD C, L\n");
        break;
      case 0x4E:
        printf("LD C, (HL)\n");
        break;
      case 0x4F:
        printf("LD C, A\n");
        break;
      case 0x50:
        printf("LD D, B\n");
        break;
      case 0x51:
        printf("LD D, C\n");
        break;
      case 0x52:
        printf("LD D, D\n");
        break;
      case 0x56:
        printf("LD D, (HL)\n");
        break;
      case 0x57:
        printf("LD D, A\n");
        break;
      case 0x5E:
        printf("LD E, (HL)\n");
        break;
      case 0x5F:
        printf("LD E, A\n");
        break;
      case 0x67:
        printf("LD H, A\n");
        break;
      case 0x6F:
        printf("LD L, A\n");
        break;
      case 0x70:
        printf("LD (HL), B\n");
        break;
      case 0x71:
        printf("LD (HL), C\n");
        break;
      case 0x72:
        printf("LD (HL), D\n");
        break;
      case 0x76:
        printf("HALT\n");
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
      case 0x7A:
        printf("LD A, D\n");
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
      case 0x7E:
        printf("LD A, (HL)\n");
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
      case 0xA7:
        printf("AND A\n");
        break;
      case 0xA9:
        printf("XOR C\n");
        break;
      case 0xAE:
        printf("XOR (HL)\n");
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
      case 0xB7:
        printf("OR A\n");
        break;
      case 0xBE:
        printf("CP (HL)\n");
        break;
      case 0xC0:
        printf("RET NZ\n");
        break;
      case 0xC1:
        printf("POP BC\n");
        break;
      case 0xC3:
        printf("JP $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xC4:
        printf("CALL NZ, $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xC5:
        printf("PUSH BC\n");
        break;
      case 0xC6:
        printf("ADD A, $%02X\n", rd8());
        PC --;
        break;
      case 0xC8:
        printf("RET Z\n");
        break;
      case 0xC9:
        printf("RET\n");
        break;
      case 0xCA:
        printf("JP Z, $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xCD:
        printf("CALL $%04X\n", rd16());
        PC -= 2;
        break;
      case 0xD1:
        printf("POP DE\n");
        break;
      case 0xD5:
        printf("PUSH DE\n");
        break;
      case 0xD6:
        printf("SUB $%02X\n", rd8());
        PC --;
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
      case 0xE5:
        printf("PUSH HL\n");
        break;
      case 0xE6:
        printf("AND $%02X\n", rd8());
        PC --;
        break;
      case 0xE9:
        printf("JP HL\n");
        break;
      case 0xEA:
        printf("LD ($%04X), A\n", rd16());
        PC -= 2;
        break;
      case 0xEE:
        printf("XOR $%02X\n", rd8());
        PC --;
        break;
      case 0xEF:
        printf("RST 5\n");
        break;
      case 0xF0:
        printf("LD A, ($FF00+$%02X)\n", rd8());
        PC --;
        break;
      case 0xF1:
        printf("POP AF\n");
        break;
      case 0xF3:
        printf("DI\n");
        break;
      case 0xF5:
        printf("PUSH AF\n");
        break;
      case 0xFA:
        printf("LD A, $%04X\n", rd16());
        PC -= 2;
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
