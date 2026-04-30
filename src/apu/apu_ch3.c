#include <stdint.h>

#include "apu_private.h"

void apu_write_ch3(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF1A:
      apu->nr30 = val;
      if (!((unsigned)val & (1U << NR52_POWER_BIT))) {
        apu->ch3_active = false;
      }
      break;
    case 0xFF1B:
      apu->nr31 = val;
      apu->ch3_len = val;
      break;
    case 0xFF1C:
      apu->nr32 = val;
      break;
    case 0xFF1D:
      apu->nr33 = val;
      break;
    case 0xFF1E: {
      bool old_len_enable = apu->ch3_len_enable;
      apu->ch3_len_enable = (((unsigned)val >> 6U) & 1U) != 0;
      if (!old_len_enable && apu->ch3_len_enable &&
          ((unsigned)apu->seq_step & 1U) == 0U) {
        if (clock_length_u16(&apu->ch3_len, APU_CH3_LEN_MAX)) {
          apu->ch3_active = false;
        }
      }
      apu->nr34 = (uint8_t)((unsigned)val & 0x7FU);
      if ((unsigned)val & (1U << NR52_POWER_BIT)) {
        if (apu->ch3_len >= APU_CH3_LEN_MAX) {
          apu->ch3_len =
              (apu->ch3_len_enable && ((unsigned)apu->seq_step & 1U) == 0U) ? 1
                                                                            : 0;
        }
        if ((unsigned)apu->nr30 & (1U << NR52_POWER_BIT)) {
          apu->ch3_active = true;
        }
        uint16_t freq3 = (uint16_t)(((unsigned)(apu->nr34 & 0x07U) << 8U) |
                                    (unsigned)apu->nr33);
        apu->ch3_freq_timer = (uint16_t)((2048U - freq3) * 2U);
        apu->ch3_pos = 0;
      }
      break;
    }
    default:
      break;
  }
}

void tick_ch3_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq =
      (uint16_t)(((unsigned)(apu->nr34 & 0x07U) << 8U) | (unsigned)apu->nr33);
  uint16_t period = (uint16_t)((2048U - freq) * 2U);
  if (period == 0) {
    period = 1;
  }
  int32_t timer_val = (int32_t)apu->ch3_freq_timer - (int32_t)cycles;
  while (timer_val <= 0) {
    apu->ch3_pos = (uint8_t)((unsigned)apu->ch3_pos + 1U) & 31U;
    timer_val += (int32_t)period;
  }
  apu->ch3_freq_timer = (uint16_t)timer_val;
}
