#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/util.h"
#include "internal.h"

static const uint16_t INTR_VECTORS[5] = {
    0x0040,  // VBlank
    0x0048,  // LCD STAT
    0x0050,  // Timer
    0x0058,  // Serial
    0x0060,  // Joypad
};

int do_intr(struct Cpu* cpu, struct Bus* bus, uint8_t intr) {
  cpu->halted = false;
  if (cpu->ime) {
    cpu->if_reg &= (uint8_t)~(1u << intr);
    push(cpu, bus, cpu->PC);
    cpu->PC = INTR_VECTORS[intr];
    cpu->ime = false;
    return 20;
  }
  return 0;
}

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  uint8_t pending = cpu->if_reg & cpu->ie_reg & 0x1Fu;
  for (uint8_t intr = 0; intr < 5; intr++) {
    if (pending & (uint8_t)(1u << intr)) {
      return (uint8_t)do_intr(cpu, bus, intr);
    }
  }
  return 0;
}

void check_interrupt_vblank_lcd(struct Bus* bus, uint8_t stat, int prev_mode,
                                int curr_mode) {
  int req_vblank = 0;
  int req_lcd = 0;
  if (prev_mode != curr_mode) {
    if (curr_mode == 1) req_vblank = 1;
    if (curr_mode == 0 && get_bit(stat, 3)) req_lcd = 1;
    if (curr_mode == 1 && get_bit(stat, 4)) req_lcd = 1;
    if (curr_mode == 2 && get_bit(stat, 5)) req_lcd = 1;
  }
  int prev_lyc = get_bit(stat, 2);
  int curr_lyc = (bus_read(bus, 0xFF44) == bus_read(bus, 0xFF45));
  if (prev_lyc != curr_lyc && curr_lyc && get_bit(stat, 6)) req_lcd = 1;
  if (req_vblank) bus_req_intr(bus, INTR_VBLANK);
  if (req_lcd) bus_req_intr(bus, INTR_LCD);
}
