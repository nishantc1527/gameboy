#pragma once
#include <stdint.h>

enum { BROM_FRAMES = 330 };

static inline uint8_t get_bit(uint8_t var, uint8_t bit) {
  return ((unsigned)var >> (unsigned)bit) & 1U;
}
static inline void set_bit(uint8_t* var, uint8_t bit) {
  *var |= (uint8_t)(1U << (unsigned)bit);
}
static inline void clear_bit(uint8_t* var, uint8_t bit) {
  *var &= (uint8_t)~(1U << (unsigned)bit);
}
