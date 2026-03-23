#include "cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"

static const uint16_t INTR_VECTORS[5] = {
    INTR_VEC_VBLANK, INTR_VEC_LCD,    INTR_VEC_TIMER,
    INTR_VEC_SERIAL, INTR_VEC_JOYPAD,
};

int do_intr(struct Cpu* cpu, struct Bus* bus, uint8_t intr) {
  cpu->halted = false;
  if (cpu->ime) {
    cpu->if_reg &= (uint8_t)~(1u << intr);
    push(cpu, bus, cpu->PC);
    cpu->PC = INTR_VECTORS[intr];
    cpu->ime = false;
    return INTR_DISPATCH_CYCLES;
  }
  return 0;
}

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  uint8_t pending = cpu->if_reg & cpu->ie_reg & IF_VALID_MASK;
  for (uint8_t intr = 0; intr < 5; intr++) {
    if (pending & (uint8_t)(1u << intr)) {
      return (uint8_t)do_intr(cpu, bus, intr);
    }
  }
  return 0;
}
