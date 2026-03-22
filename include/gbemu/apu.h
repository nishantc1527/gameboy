#pragma once

#include <stdint.h>

#define APU_SAMPLE_RATE 48000
#define APU_BUF_SIZE 1024

struct Apu {
  uint8_t nr10, nr11, nr12, nr13, nr14;
  uint8_t nr21, nr22, nr23, nr24;
  uint8_t nr30, nr31, nr32, nr33, nr34;
  uint8_t nr41, nr42, nr43, nr44;
  uint8_t nr50, nr51;
  uint8_t wave_ram[16];
  bool powered;
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
  bool ch2_active;
  uint8_t ch2_len;
  bool ch2_len_enable;
  uint16_t ch2_freq_timer;
  uint8_t ch2_duty_pos;
  uint8_t ch2_env_vol;
  uint8_t ch2_env_timer;
  bool ch3_active;
  uint16_t ch3_len;
  bool ch3_len_enable;
  uint16_t ch3_freq_timer;
  uint8_t ch3_pos;
  bool ch4_active;
  uint8_t ch4_len;
  bool ch4_len_enable;
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

struct Apu* apu_init(void);
void apu_free(struct Apu* apu);
uint8_t apu_read(const struct Apu* apu, uint16_t addr);
void apu_write(struct Apu* apu, uint16_t addr, uint8_t val);
void apu_notify_div_tick(struct Apu* apu);
void apu_tick(struct Apu* apu, uint8_t cycles);
