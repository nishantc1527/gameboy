#pragma once
#include <stdint.h>

static inline uint8_t gt(uint8_t var, uint8_t bt) { return (var >> bt) & 1; }
static inline void st(uint8_t* var, uint8_t bt) { *var |= (uint8_t)(1 << bt); }
static inline void cl(uint8_t* var, uint8_t bt) { *var &= (uint8_t)~(1 << bt); }
