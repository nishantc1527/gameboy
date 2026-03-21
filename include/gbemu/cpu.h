#pragma once

#include <stdint.h>

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  uint8_t bHALT, bIME, bIME_pending, bHALT_BUG;
  uint8_t if_reg;
  uint8_t ie_reg;
  uint8_t cyc_ext;
  uint8_t cgb_mode;
};

struct Bus;

struct Cpu* init_cpu(void);
void post_boot_cpu(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
int step(struct Cpu* cpu, struct Bus* bus, uint8_t disassemble_enable,
         int test_category, uint8_t* b_done, const uint16_t* watch_addrs,
         uint8_t watch_count, uint64_t total_cycles);

int check_interrupt(struct Cpu* cpu, struct Bus* bus);
void check_interrupt_vblank_lcd(struct Bus* bus, uint8_t stat, int prev_mode,
                                int curr_mode);
