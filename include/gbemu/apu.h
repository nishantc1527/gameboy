#pragma once

#include <stdint.h>

#include "gbemu/mmu.h"

#define APU_SAMPLE_RATE 48000
#define APU_BUF_SIZE 1024

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
  uint16_t ch1_freq_timer;
  uint16_t ch2_freq_timer;
  uint16_t ch3_freq_timer;
  uint32_t ch4_freq_timer;
  uint8_t ch1_duty_pos;
  uint8_t ch2_duty_pos;
  uint8_t ch3_pos;
  uint16_t ch4_lfsr;
  uint8_t ch1_env_vol;
  uint8_t ch2_env_vol;
  uint8_t ch4_env_vol;
  uint8_t ch1_env_timer;
  uint8_t ch2_env_timer;
  uint8_t ch4_env_timer;
  uint8_t envelope_clock;
  float sample_buf[APU_BUF_SIZE][2];
  uint32_t sample_count;
  uint32_t sample_acc;
  double capacitor_L;
  double capacitor_R;
};

struct Apu* init_apu();
void upd_apu(struct Apu* apu, struct Mmu* mmu, uint8_t cycles);
void reset_apu(struct Apu* apu, struct Mmu* mmu);
