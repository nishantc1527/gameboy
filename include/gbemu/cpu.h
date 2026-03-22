#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  bool halted;
  bool ime;
  bool ime_pending;
  bool halt_bug;
  uint8_t if_reg;
  uint8_t ie_reg;
  uint8_t cyc_ext;
  uint8_t cgb_mode;
  bool ldbb_fired;
};

struct Bus;

struct Cpu* init_cpu(void);
void post_boot_cpu(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus);

int check_interrupt(struct Cpu* cpu, struct Bus* bus);
void check_interrupt_vblank_lcd(struct Bus* bus, uint8_t stat, int prev_mode,
                                int curr_mode);
