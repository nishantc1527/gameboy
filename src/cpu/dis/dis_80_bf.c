#include <stdint.h>
#include <stdio.h>

#include "gbemu/bus.h"
#include "gbemu/cpu.h"

void dis_80_bf(struct Cpu* cpu, struct Bus* bus, uint8_t instr) {
  (void)cpu;
  (void)bus;
  switch (instr) {
    case 0x80:
      printf("ADD A, B\n");
      break;
    case 0x81:
      printf("ADD A, C\n");
      break;
    case 0x82:
      printf("ADD A, D\n");
      break;
    case 0x83:
      printf("ADD A, E\n");
      break;
    case 0x84:
      printf("ADD A, H\n");
      break;
    case 0x85:
      printf("ADD A, L\n");
      break;
    case 0x86:
      printf("ADD A, (HL)\n");
      break;
    case 0x87:
      printf("ADD A, A\n");
      break;
    case 0x88:
      printf("ADC A, B\n");
      break;
    case 0x89:
      printf("ADC A, C\n");
      break;
    case 0x8A:
      printf("ADC A, D\n");
      break;
    case 0x8B:
      printf("ADC A, E\n");
      break;
    case 0x8C:
      printf("ADC A, H\n");
      break;
    case 0x8D:
      printf("ADC A, L\n");
      break;
    case 0x8E:
      printf("ADC A, (HL)\n");
      break;
    case 0x8F:
      printf("ADC A, A\n");
      break;
    case 0x90:
      printf("SUB B\n");
      break;
    case 0x91:
      printf("SUB C\n");
      break;
    case 0x92:
      printf("SUB D\n");
      break;
    case 0x93:
      printf("SUB E\n");
      break;
    case 0x94:
      printf("SUB H\n");
      break;
    case 0x95:
      printf("SUB L\n");
      break;
    case 0x96:
      printf("SUB (HL)\n");
      break;
    case 0x97:
      printf("SUB A\n");
      break;
    case 0x98:
      printf("SBC A, B\n");
      break;
    case 0x99:
      printf("SBC A, C\n");
      break;
    case 0x9A:
      printf("SBC A, D\n");
      break;
    case 0x9B:
      printf("SBC A, E\n");
      break;
    case 0x9C:
      printf("SBC A, H\n");
      break;
    case 0x9D:
      printf("SBC A, L\n");
      break;
    case 0x9E:
      printf("SBC A, (HL)\n");
      break;
    case 0x9F:
      printf("SBC A, A\n");
      break;
    case 0xA0:
      printf("AND B\n");
      break;
    case 0xA1:
      printf("AND C\n");
      break;
    case 0xA2:
      printf("AND D\n");
      break;
    case 0xA3:
      printf("AND E\n");
      break;
    case 0xA4:
      printf("AND H\n");
      break;
    case 0xA5:
      printf("AND L\n");
      break;
    case 0xA6:
      printf("AND (HL)\n");
      break;
    case 0xA7:
      printf("AND A\n");
      break;
    case 0xA8:
      printf("XOR B\n");
      break;
    case 0xA9:
      printf("XOR C\n");
      break;
    case 0xAA:
      printf("XOR D\n");
      break;
    case 0xAB:
      printf("XOR E\n");
      break;
    case 0xAC:
      printf("XOR H\n");
      break;
    case 0xAD:
      printf("XOR L\n");
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
    case 0xB2:
      printf("OR D\n");
      break;
    case 0xB3:
      printf("OR E\n");
      break;
    case 0xB4:
      printf("OR H\n");
      break;
    case 0xB5:
      printf("OR L\n");
      break;
    case 0xB6:
      printf("OR (HL)\n");
      break;
    case 0xB7:
      printf("OR A\n");
      break;
    case 0xB8:
      printf("CP B\n");
      break;
    case 0xB9:
      printf("CP C\n");
      break;
    case 0xBA:
      printf("CP D\n");
      break;
    case 0xBB:
      printf("CP E\n");
      break;
    case 0xBC:
      printf("CP H\n");
      break;
    case 0xBD:
      printf("CP L\n");
      break;
    case 0xBE:
      printf("CP (HL)\n");
      break;
    case 0xBF:
      printf("CP A\n");
      break;
    default:
      printf("UNKNOWN INSTRUCTION %02X\n", instr);
      break;
  }
}
