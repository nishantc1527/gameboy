#include "gbemu/cpu.h"

#include <stdint.h>
#include <stdlib.h>

struct Cpu* init_cpu(void) {
  struct Cpu* cpu = calloc(1, sizeof(struct Cpu));
  cpu->PC = 0x0000;
  cpu->cgb_mode = 0;
  return cpu;
}

void post_boot_cpu(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum) {
  if (cgb_mode) {
    cpu->A = 0x11;
    cpu->F = 0x80;
    cpu->B = 0x00;
    cpu->C = 0x00;
    cpu->D = 0xFF;
    cpu->E = 0x56;
    cpu->H = 0x00;
    cpu->L = 0x0D;
  } else {
    cpu->A = 0x01;
    cpu->F = (header_checksum != 0) ? 0xB0 : 0x80;
    cpu->B = 0x00;
    cpu->C = 0x13;
    cpu->D = 0x00;
    cpu->E = 0xD8;
    cpu->H = 0x01;
    cpu->L = 0x4D;
  }
  cpu->PC = 0x0100;
  cpu->SP = 0xFFFE;
  cpu->if_reg = 0xE1;
  cpu->ie_reg = 0x00;
}
