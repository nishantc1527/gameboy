#include "gbemu/timer.h"

#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"

#define REG_DIV 0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA 0xFF06
#define REG_TAC 0xFF07

#define TAC_ENABLE_BIT 2
#define TAC_CLOCK_MASK 0x03
#define TAC_VALID_MASK 0x07
#define TAC_UNUSED_BITS 0xF8

#define TIMER_CLK0_BIT 9
#define TIMER_CLK1_BIT 3
#define TIMER_CLK2_BIT 5
#define TIMER_CLK3_BIT 7
#define TIMER_APU_BIT 12

static uint8_t timer_selected_bit(const struct Timer* t) {
  switch (t->tac & TAC_CLOCK_MASK) {
    case 0:
      return TIMER_CLK0_BIT;
    case 1:
      return TIMER_CLK1_BIT;
    case 2:
      return TIMER_CLK2_BIT;
    case 3:
      return TIMER_CLK3_BIT;
    default:
      return TIMER_CLK0_BIT;
  }
}

static void timer_increment_tima(struct Timer* t) {
  if (t->tima == 0xFF) {
    t->tima = 0x00;
    t->tima_overflow_pending = true;
  } else
    t->tima++;
}

struct Timer* timer_init(void) { return calloc(1, sizeof(struct Timer)); }

void timer_free(struct Timer* t) { free(t); }

void timer_post_boot(struct Timer* t) {
  t->sys_ctr = 0x0000;
  t->tima = 0x00;
  t->tma = 0x00;
  t->tac = 0x00;
  t->tima_overflow_pending = false;
}

uint8_t timer_read(const struct Timer* t, uint16_t addr) {
  if (addr == REG_DIV) return (uint8_t)(t->sys_ctr >> 8);
  if (addr == REG_TIMA) return t->tima;
  if (addr == REG_TMA) return t->tma;
  if (addr == REG_TAC) return t->tac | TAC_UNUSED_BITS;
  return BUS_OPEN_BUS;
}

void timer_write(struct Timer* t, uint16_t addr, uint8_t val, struct Apu* apu,
                 struct Cpu* cpu) {
  if (addr == REG_DIV) {
    uint8_t sel_bit = timer_selected_bit(t);
    if ((t->tac & (1u << TAC_ENABLE_BIT)) && ((t->sys_ctr >> sel_bit) & 1)) {
      timer_increment_tima(t);
    }
    if ((t->sys_ctr >> TIMER_APU_BIT) & 1) {
      apu_notify_div_tick(apu);
    }
    t->sys_ctr = 0;
    return;
  }
  if (addr == REG_TIMA) {
    t->tima = val;
    t->tima_overflow_pending = false;
    return;
  }
  if (addr == REG_TMA) {
    t->tma = val;
    return;
  }
  if (addr == REG_TAC) {
    uint8_t old_bit = timer_selected_bit(t);
    uint8_t old_enabled = (t->tac & (1u << TAC_ENABLE_BIT)) != 0;
    t->tac = val & TAC_VALID_MASK;
    uint8_t new_enabled = (t->tac & (1u << TAC_ENABLE_BIT)) != 0;
    if (old_enabled && (t->sys_ctr >> old_bit) & 1) {
      uint8_t new_bit = timer_selected_bit(t);
      uint8_t new_bit_is_1 = (t->sys_ctr >> new_bit) & 1;
      if (!new_enabled || !new_bit_is_1) {
        timer_increment_tima(t);
      }
    }
    return;
  }
}

void timer_tick(struct Timer* t, uint8_t cycles, struct Apu* apu,
                struct Cpu* cpu) {
  if (t->tima_overflow_pending) {
    t->tima_overflow_pending = false;
    t->tima = t->tma;
    cpu->if_reg |= (uint8_t)(1u << INTR_TIMER);
  }

  for (uint8_t i = 0; i < cycles; i++) {
    uint16_t old = t->sys_ctr;
    t->sys_ctr = (uint16_t)(t->sys_ctr + 1);
    if ((old >> TIMER_APU_BIT) & 1 && !((t->sys_ctr >> TIMER_APU_BIT) & 1)) {
      apu_notify_div_tick(apu);
    }
    if (t->tac & (1u << TAC_ENABLE_BIT)) {
      uint8_t bit = timer_selected_bit(t);
      if ((old >> bit) & 1 && !((t->sys_ctr >> bit) & 1)) {
        timer_increment_tima(t);
      }
    }
  }
}
