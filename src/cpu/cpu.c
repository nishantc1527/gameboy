#include "gbemu/cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu_private.h"
#include "gbemu/bus.h"

struct Cpu* cpu_init(bool cgb_mode) {
  struct Cpu* cpu = calloc(1, sizeof(struct Cpu));
  cpu->PC = 0x0000;
  cpu->cgb_mode = cgb_mode;
  return cpu;
}

uint8_t cpu_read_if(const struct Cpu* cpu) {
  return (uint8_t)((unsigned)cpu->if_reg | IF_UNUSED_BITS);
}

void cpu_write_if(struct Cpu* cpu, uint8_t val) {
  cpu->if_reg = (uint8_t)((unsigned)val & IF_VALID_MASK);
}

void cpu_req_intr(struct Cpu* cpu, uint8_t intr) {
  cpu->if_reg |= (uint8_t)(1U << intr);
}

uint8_t cpu_read_ie(const struct Cpu* cpu) { return cpu->ie_reg; }

void cpu_write_ie(struct Cpu* cpu, uint8_t val) { cpu->ie_reg = val; }

bool cpu_get_cgb_mode(const struct Cpu* cpu) { return cpu->cgb_mode; }

bool cpu_check_ld_b_b(struct Cpu* cpu) {
  bool fired = cpu->ld_b_b_fired;
  cpu->ld_b_b_fired = false;
  return fired;
}

uint8_t cpu_get_b(const struct Cpu* cpu) { return cpu->B; }
uint8_t cpu_get_c(const struct Cpu* cpu) { return cpu->C; }
uint8_t cpu_get_d(const struct Cpu* cpu) { return cpu->D; }
uint8_t cpu_get_e(const struct Cpu* cpu) { return cpu->E; }
uint8_t cpu_get_h(const struct Cpu* cpu) { return cpu->H; }
uint8_t cpu_get_l(const struct Cpu* cpu) { return cpu->L; }
