#pragma once

#include <stdint.h>

#include "gbemu/mmu.h"

struct Apu {
  uint8_t ch1_enable, ch2_enable, ch3_enable, ch4_enable;
  uint8_t ch1_len_enable, ch1_len, ch1_len_apu_div, ch1_len_init;
  uint8_t ch2_len_enable, ch2_len, ch2_len_apu_div, ch2_len_init;
  uint8_t ch3_len_enable, ch3_len_apu_div;
  uint16_t ch3_len, ch3_len_init;
  uint8_t ch4_len_enable, ch4_len, ch4_len_apu_div, ch4_len_init;
  uint8_t div_apu;
};

struct Apu* init_apu();
void upd_apu(struct Apu* apu, struct Mmu* mmu);

void reset_apu(struct Apu* apu, struct Mmu* mmu);
