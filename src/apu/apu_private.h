#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/util.h"

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
