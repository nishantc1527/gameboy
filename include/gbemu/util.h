#pragma once
#include <stdint.h>

#define BROM_FRAMES 330

static inline uint8_t get_bit(uint8_t var, uint8_t bit) {
  return (var >> bit) & 1;
}
static inline void set_bit(uint8_t* var, uint8_t bit) {
  *var |= (uint8_t)(1 << bit);
}
static inline void clear_bit(uint8_t* var, uint8_t bit) {
  *var &= (uint8_t)~(1 << bit);
}
