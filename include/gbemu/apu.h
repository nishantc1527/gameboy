#pragma once

#include <stdint.h>

#include "gbemu/mmu.h"

struct Apu {
  uint8_t ch1_enable, ch2_enable, ch3_enable, ch4_enable;
  uint8_t ch1_len, ch2_len, ch4_len;
  uint16_t ch3_len;
  uint8_t ch1_len_enable, ch2_len_enable, ch3_len_enable, ch4_len_enable;
  uint8_t div_apu;
  uint8_t length_clock;
  uint8_t sweep_clock;
  uint8_t apu_on;
  uint16_t sweep_freq;
  uint8_t sweep_timer;
  uint8_t sweep_enable;
  uint8_t sweep_neg_used;
  uint8_t ch_len_dirty;
};

struct Apu* init_apu();
void upd_apu(struct Apu* apu, struct Mmu* mmu);
void reset_apu(struct Apu* apu, struct Mmu* mmu);
