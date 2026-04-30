#include "gbemu/timer.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"

enum {
  REG_DIV = 0xFF04,
  REG_TIMA = 0xFF05,
  REG_TMA = 0xFF06,
  REG_TAC = 0xFF07
};

enum {
  TAC_ENABLE_BIT = 2,
  TAC_CLOCK_MASK = 0x03,
  TAC_VALID_MASK = 0x07,
  TAC_UNUSED_BITS = 0xF8
};

enum {
  TIMER_CLK0_BIT = 9,
  TIMER_CLK1_BIT = 3,
  TIMER_CLK2_BIT = 5,
  TIMER_CLK3_BIT = 7,
  TIMER_APU_BIT = 12
};

struct Timer {
  uint16_t sys_ctr;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;
  uint8_t tima_overflow_pending;
  uint8_t sub_instr_cycles;
};

static uint8_t timer_selected_bit(const struct Timer* tmr) {
  switch ((unsigned)tmr->tac & TAC_CLOCK_MASK) {
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

static void timer_increment_tima(struct Timer* tmr) {
  if (tmr->tima == 0xFF) {
    tmr->tima = 0x00;
    tmr->tima_overflow_pending = 1U;
  } else {
    {
      tmr->tima++;
    }
  }
}

struct Timer* timer_init(void) { return calloc(1, sizeof(struct Timer)); }

void timer_free(struct Timer* tmr) { free(tmr); }

uint8_t timer_read(const struct Timer* tmr, uint16_t addr) {
  if (addr == REG_DIV) {
    return (uint8_t)((unsigned)tmr->sys_ctr >> 8U);
  }
  if (addr == REG_TIMA) {
    return tmr->tima;
  }
  if (addr == REG_TMA) {
    return tmr->tma;
  }
  if (addr == REG_TAC) {
    return (uint8_t)((unsigned)tmr->tac | TAC_UNUSED_BITS);
  }
  return BUS_OPEN_BUS;
}

void timer_write(struct Timer* tmr, uint16_t addr, uint8_t val,
                 struct Bus* bus) {
  if (addr == REG_DIV) {
    const uint8_t sel_bit = timer_selected_bit(tmr);
    if (((unsigned)tmr->tac & (1U << TAC_ENABLE_BIT)) &&
        (((unsigned)tmr->sys_ctr >> sel_bit) & 1U)) {
      timer_increment_tima(tmr);
    }
    if (((unsigned)tmr->sys_ctr >> TIMER_APU_BIT) & 1U) {
      bus_notify_div_pulse(bus);
    }
    tmr->sys_ctr = 0;
    return;
  }
  if (addr == REG_TIMA) {
    tmr->tima = val;
    tmr->tima_overflow_pending = 0U;
    return;
  }
  if (addr == REG_TMA) {
    tmr->tma = val;
    return;
  }
  if (addr == REG_TAC) {
    const uint8_t old_bit = timer_selected_bit(tmr);
    const uint8_t old_enabled =
        ((unsigned)tmr->tac & (1U << TAC_ENABLE_BIT)) != 0;
    tmr->tac = (uint8_t)((unsigned)val & TAC_VALID_MASK);
    const uint8_t new_enabled =
        ((unsigned)tmr->tac & (1U << TAC_ENABLE_BIT)) != 0;
    if (old_enabled && (((unsigned)tmr->sys_ctr >> old_bit) & 1U)) {
      const uint8_t new_bit = timer_selected_bit(tmr);
      const uint8_t new_bit_is_1 =
          (uint8_t)(((unsigned)tmr->sys_ctr >> new_bit) & 1U);
      if (!new_enabled || !new_bit_is_1) {
        timer_increment_tima(tmr);
      }
    }
  }
}

uint8_t timer_consume_sub_cycles(struct Timer* tmr) {
  const uint8_t sub_cyc = tmr->sub_instr_cycles;
  tmr->sub_instr_cycles = 0;
  return sub_cyc;
}

void timer_record_sub_cycles(struct Timer* tmr, uint8_t cycles) {
  tmr->sub_instr_cycles = (uint8_t)(tmr->sub_instr_cycles + cycles);
}

void timer_tick(struct Timer* tmr, uint8_t cycles, struct Bus* bus) {
  if (tmr->tima_overflow_pending) {
    tmr->tima_overflow_pending = 0U;
    tmr->tima = tmr->tma;
    bus_req_intr(bus, INTR_TIMER);
  }
  for (uint8_t i = 0; i < cycles; i++) {
    const uint16_t old = tmr->sys_ctr;
    tmr->sys_ctr = (uint16_t)(tmr->sys_ctr + 1U);
    if ((((unsigned)old >> TIMER_APU_BIT) & 1U) &&
        !(((unsigned)tmr->sys_ctr >> TIMER_APU_BIT) & 1U)) {
      bus_notify_div_pulse(bus);
    }
    if ((unsigned)tmr->tac & (1U << TAC_ENABLE_BIT)) {
      const uint8_t bit = timer_selected_bit(tmr);
      if ((((unsigned)old >> bit) & 1U) &&
          !(((unsigned)tmr->sys_ctr >> bit) & 1U)) {
        timer_increment_tima(tmr);
      }
    }
  }
}
