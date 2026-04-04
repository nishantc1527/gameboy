#pragma once
#include <stdint.h>

#define APU_SAMPLE_RATE 48000
#define APU_BUF_SIZE 1024

struct Apu;

struct Apu* apu_init(void);
void apu_free(struct Apu* apu);
uint8_t apu_read(const struct Apu* apu, uint16_t addr);
void apu_write(struct Apu* apu, uint16_t addr, uint8_t val);
void apu_notify_div_tick(struct Apu* apu);
void apu_tick(struct Apu* apu, uint8_t cycles);
void apu_discard_samples(struct Apu* apu);
uint32_t apu_get_sample_count(const struct Apu* apu);
const float (*apu_get_sample_buf(const struct Apu* apu))[2];
