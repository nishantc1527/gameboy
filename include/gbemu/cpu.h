#pragma once
#include <stdint.h>

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  uint8_t halted;
  uint8_t ime;
  uint8_t ime_pending;
  uint8_t halt_bug;
  uint8_t if_reg;
  uint8_t ie_reg;
  uint8_t cgb_mode;
  uint8_t ldbb_fired;
};

struct Bus;

struct Cpu* init_cpu(void);
void post_boot_cpu(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus);
void check_interrupt_vblank_lcd(struct Bus* bus, uint8_t stat, int prev_mode,
                                int curr_mode);
