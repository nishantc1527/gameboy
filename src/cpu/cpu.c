#include "gbemu/cpu.h"

#include <stdint.h>
#include <stdlib.h>

#include "cpu_private.h"

struct Cpu* cpu_init(uint8_t cgb_mode) {
  struct Cpu* cpu = calloc(1, sizeof(struct Cpu));
  cpu->PC = 0x0000;
  cpu->cgb_mode = cgb_mode;
  return cpu;
}

uint8_t cpu_read_if(const struct Cpu* cpu) {
  return cpu->if_reg | IF_UNUSED_BITS;
}

void cpu_write_if(struct Cpu* cpu, uint8_t val) {
  cpu->if_reg = val & IF_VALID_MASK;
}

uint8_t cpu_read_ie(const struct Cpu* cpu) { return cpu->ie_reg; }

void cpu_write_ie(struct Cpu* cpu, uint8_t val) { cpu->ie_reg = val; }

void cpu_post_boot(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum) {
  if (cgb_mode) {
    cpu->A = CPU_CGB_A;
    cpu->F = CPU_CGB_F;
    cpu->B = CPU_DMG_B;
    cpu->C = 0x00;
    cpu->D = CPU_CGB_D;
    cpu->E = CPU_CGB_E;
    cpu->H = CPU_CGB_H;
    cpu->L = CPU_CGB_L;
  } else {
    cpu->A = CPU_DMG_A;
    cpu->F =
        (header_checksum != 0) ? CPU_DMG_F_WITH_CHKSUM : CPU_DMG_F_NO_CHKSUM;
    cpu->B = CPU_DMG_B;
    cpu->C = CPU_DMG_C;
    cpu->D = CPU_DMG_D;
    cpu->E = CPU_DMG_E;
    cpu->H = CPU_DMG_H;
    cpu->L = CPU_DMG_L;
  }
  cpu->PC = CPU_BOOT_PC;
  cpu->SP = CPU_BOOT_SP;
  cpu->if_reg = CPU_BOOT_IF;
  cpu->ie_reg = 0x00;
}
