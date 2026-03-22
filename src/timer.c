#include "gbemu/timer.h"

#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"

#define INTR_TIMER 2

static uint8_t timer_selected_bit(const struct Timer* t) {
  switch (t->tac & 0x03) {
    case 0:
      return 9;  // 4096 Hz
    case 1:
      return 3;  // 262144 Hz
    case 2:
      return 5;  // 65536 Hz
    case 3:
      return 7;  // 16384 Hz
    default:
      return 9;
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
  if (addr == 0xFF04) return (uint8_t)(t->sys_ctr >> 8);
  if (addr == 0xFF05) return t->tima;
  if (addr == 0xFF06) return t->tma;
  if (addr == 0xFF07) return t->tac | 0xF8;
  return 0xFF;
}

void timer_write(struct Timer* t, uint16_t addr, uint8_t val, struct Apu* apu,
                 struct Cpu* cpu) {
  if (addr == 0xFF04) {
    uint8_t sel_bit = timer_selected_bit(t);
    if ((t->tac & 0x04) && ((t->sys_ctr >> sel_bit) & 1)) {
      timer_increment_tima(t);
    }
    if ((t->sys_ctr >> 12) & 1) {
      apu_notify_div_tick(apu);
    }
    t->sys_ctr = 0;
    return;
  }
  if (addr == 0xFF05) {
    t->tima = val;
    t->tima_overflow_pending = false;
    return;
  }
  if (addr == 0xFF06) {
    t->tma = val;
    return;
  }
  if (addr == 0xFF07) {
    uint8_t old_bit = timer_selected_bit(t);
    uint8_t old_enabled = (t->tac & 0x04) != 0;
    t->tac = val & 0x07u;
    uint8_t new_enabled = (t->tac & 0x04) != 0;
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
    if ((old >> 12) & 1 && !((t->sys_ctr >> 12) & 1)) {
      apu_notify_div_tick(apu);
    }
    if (t->tac & 0x04) {
      uint8_t bit = timer_selected_bit(t);
      if ((old >> bit) & 1 && !((t->sys_ctr >> bit) & 1)) {
        timer_increment_tima(t);
      }
    }
  }
}
