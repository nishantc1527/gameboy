#pragma once
#include <stdint.h>

struct Apu;
struct Cpu;

struct Timer {
  uint16_t sys_ctr;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;
  uint8_t tima_overflow_pending;
  uint8_t sub_instr_cycles;
};

struct Timer* timer_init(void);
void timer_free(struct Timer* t);
void timer_post_boot(struct Timer* t);
uint8_t timer_read(const struct Timer* t, uint16_t addr);
void timer_write(struct Timer* t, uint16_t addr, uint8_t val, struct Apu* apu,
                 struct Cpu* cpu);
void timer_tick(struct Timer* t, uint8_t cycles, struct Apu* apu,
                struct Cpu* cpu);
