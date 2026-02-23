#pragma once
#include <stdint.h>

static inline uint8_t gb(uint8_t var, uint8_t bt) { return (var >> bt) & 1; }
static inline void sb(uint8_t* var, uint8_t bt) {
  *var |= (uint8_t)(1 << bt);
}
static inline void cb(uint8_t* var, uint8_t bt) {
  *var &= (uint8_t)~(1 << bt);
}
