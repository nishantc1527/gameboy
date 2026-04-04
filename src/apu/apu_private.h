#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/util.h"

struct Apu {
  uint8_t nr10, nr11, nr12, nr13, nr14;
  uint8_t nr21, nr22, nr23, nr24;
  uint8_t nr30, nr31, nr32, nr33, nr34;
  uint8_t nr41, nr42, nr43, nr44;
  uint8_t nr50, nr51;
  uint8_t wave_ram[16];
  uint8_t powered;
  uint8_t seq_step;
  uint8_t ch1_active;
  uint8_t ch1_len;
  uint8_t ch1_len_enable;
  uint16_t ch1_freq_timer;
  uint8_t ch1_duty_pos;
  uint16_t sweep_freq;
  uint8_t sweep_timer;
  uint8_t sweep_enable;
  uint8_t sweep_neg_used;
  uint8_t ch1_env_vol;
  uint8_t ch1_env_timer;
  uint8_t ch2_active;
  uint8_t ch2_len;
  uint8_t ch2_len_enable;
  uint16_t ch2_freq_timer;
  uint8_t ch2_duty_pos;
  uint8_t ch2_env_vol;
  uint8_t ch2_env_timer;
  uint8_t ch3_active;
  uint16_t ch3_len;
  uint8_t ch3_len_enable;
  uint16_t ch3_freq_timer;
  uint8_t ch3_pos;
  uint8_t ch4_active;
  uint8_t ch4_len;
  uint8_t ch4_len_enable;
  uint32_t ch4_freq_timer;
  uint16_t ch4_lfsr;
  uint8_t ch4_env_vol;
  uint8_t ch4_env_timer;
  float sample_buf[APU_BUF_SIZE][2];
  uint32_t sample_count;
  uint32_t sample_acc;
  double capacitor_L;
  double capacitor_R;
};

#define HPF_CHARGE 0.99634f

#define APU_CH1_LEN_MAX 64
#define APU_CH2_LEN_MAX 64
#define APU_CH3_LEN_MAX 256
#define APU_CH4_LEN_MAX 64
#define APU_LFSR_INIT 0x7FFF

#define NR52_POWER_BIT 7
#define NR52_UNUSED_BITS 0x70

#define NR10_UNUSED 0x80
#define NR11_UNUSED 0x3F
#define NR13_UNUSED 0xFF
#define NR14_UNUSED 0xBF
#define NR21_UNUSED 0x3F
#define NR23_UNUSED 0xFF
#define NR24_UNUSED 0xBF
#define NR30_UNUSED 0x7F
#define NR31_UNUSED 0xFF
#define NR32_UNUSED 0x9F
#define NR33_UNUSED 0xFF
#define NR34_UNUSED 0xBF
#define NR41_UNUSED 0xC0
#define NR44_UNUSED 0xBF

static inline uint8_t clock_length_u8(uint8_t* len, uint8_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}

static inline uint8_t clock_length_u16(uint16_t* len, uint16_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}

void apu_write_ch1(struct Apu* apu, uint16_t addr, uint8_t val);
void apu_write_ch2(struct Apu* apu, uint16_t addr, uint8_t val);
void apu_write_ch3(struct Apu* apu, uint16_t addr, uint8_t val);
void apu_write_ch4(struct Apu* apu, uint16_t addr, uint8_t val);

void tick_ch1_freq(struct Apu* apu, uint8_t cycles);
void tick_ch2_freq(struct Apu* apu, uint8_t cycles);
void tick_ch3_freq(struct Apu* apu, uint8_t cycles);
void tick_ch4_freq(struct Apu* apu, uint8_t cycles);

void apu_clock_sweep(struct Apu* apu);
void apu_mix_samples(struct Apu* apu, uint8_t cycles);
