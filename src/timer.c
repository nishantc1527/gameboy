#include "gbemu/timer.h"

#include <stdlib.h>

#include "gbemu/bus.h"

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

struct Timer {
  uint16_t sys_ctr;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;
  uint8_t tima_overflow_pending;
  uint8_t sub_instr_cycles;
};

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

uint8_t timer_read(const struct Timer* t, uint16_t addr) {
  if (addr == REG_DIV) return (uint8_t)(t->sys_ctr >> 8);
  if (addr == REG_TIMA) return t->tima;
  if (addr == REG_TMA) return t->tma;
  if (addr == REG_TAC) return t->tac | TAC_UNUSED_BITS;
  return BUS_OPEN_BUS;
}

void timer_write(struct Timer* t, uint16_t addr, uint8_t val, struct Bus* bus) {
  if (addr == REG_DIV) {
    const uint8_t sel_bit = timer_selected_bit(t);
    if ((t->tac & (1u << TAC_ENABLE_BIT)) && ((t->sys_ctr >> sel_bit) & 1)) {
      timer_increment_tima(t);
    }
    if ((t->sys_ctr >> TIMER_APU_BIT) & 1) {
      bus_notify_div_pulse(bus);
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
    const uint8_t old_bit = timer_selected_bit(t);
    const uint8_t old_enabled = (t->tac & (1u << TAC_ENABLE_BIT)) != 0;
    t->tac = val & TAC_VALID_MASK;
    const uint8_t new_enabled = (t->tac & (1u << TAC_ENABLE_BIT)) != 0;
    if (old_enabled && (t->sys_ctr >> old_bit) & 1) {
      const uint8_t new_bit = timer_selected_bit(t);
      const uint8_t new_bit_is_1 = (t->sys_ctr >> new_bit) & 1;
      if (!new_enabled || !new_bit_is_1) {
        timer_increment_tima(t);
      }
    }
  }
}

uint8_t timer_consume_sub_cycles(struct Timer* t) {
  const uint8_t v = t->sub_instr_cycles;
  t->sub_instr_cycles = 0;
  return v;
}

void timer_record_sub_cycles(struct Timer* t, uint8_t cycles) {
  t->sub_instr_cycles = (uint8_t)(t->sub_instr_cycles + cycles);
}

void timer_tick(struct Timer* t, uint8_t cycles, struct Bus* bus) {
  if (t->tima_overflow_pending) {
    t->tima_overflow_pending = false;
    t->tima = t->tma;
    bus_req_intr(bus, INTR_TIMER);
  }
  for (uint8_t i = 0; i < cycles; i++) {
    const uint16_t old = t->sys_ctr;
    t->sys_ctr = (uint16_t)(t->sys_ctr + 1);
    if ((old >> TIMER_APU_BIT) & 1 && !((t->sys_ctr >> TIMER_APU_BIT) & 1)) {
      bus_notify_div_pulse(bus);
    }
    if (t->tac & (1u << TAC_ENABLE_BIT)) {
      const uint8_t bit = timer_selected_bit(t);
      if ((old >> bit) & 1 && !((t->sys_ctr >> bit) & 1)) {
        timer_increment_tima(t);
      }
    }
  }
}
