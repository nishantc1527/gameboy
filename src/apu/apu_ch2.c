#include <stdint.h>

#include "apu_private.h"

void apu_write_ch2(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF16:
      apu->nr21 = val;
      apu->ch2_len = (uint8_t)((unsigned)val & (APU_CH2_LEN_MAX - 1U));
      break;
    case 0xFF17:
      apu->nr22 = val;
      if (!((unsigned)val & 0xF8U)) {
        apu->ch2_active = false;
      }
      break;
    case 0xFF18:
      apu->nr23 = val;
      break;
    case 0xFF19: {
      bool old_len_enable = apu->ch2_len_enable;
      apu->ch2_len_enable = (((unsigned)val >> 6U) & 1U) != 0;
      if (!old_len_enable && apu->ch2_len_enable &&
          ((unsigned)apu->seq_step & 1U) == 0U) {
        if (clock_length_u8(&apu->ch2_len, APU_CH2_LEN_MAX)) {
          apu->ch2_active = false;
        }
      }
      apu->nr24 = (uint8_t)((unsigned)val & 0x7FU);
      if ((unsigned)val & (1U << NR52_POWER_BIT)) {
        if (apu->ch2_len >= APU_CH2_LEN_MAX) {
          apu->ch2_len =
              (apu->ch2_len_enable && ((unsigned)apu->seq_step & 1U) == 0U) ? 1
                                                                            : 0;
        }
        if ((unsigned)apu->nr22 & 0xF8U) {
          apu->ch2_active = true;
        }
        uint16_t freq2 = (uint16_t)(((unsigned)(apu->nr24 & 0x07U) << 8U) |
                                    (unsigned)apu->nr23);
        apu->ch2_freq_timer = (uint16_t)((2048U - freq2) * 4U);
        apu->ch2_env_vol = (uint8_t)(((unsigned)apu->nr22 >> 4U) & 0x0FU);
        uint8_t ep2 = (uint8_t)((unsigned)apu->nr22 & 0x07U);
        apu->ch2_env_timer = ep2 ? ep2 : 8;
      }
      break;
    }
    default:
      break;
  }
}

void tick_ch2_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq =
      (uint16_t)(((unsigned)(apu->nr24 & 0x07U) << 8U) | (unsigned)apu->nr23);
  uint16_t period = (uint16_t)((2048U - freq) * 4U);
  if (period == 0) {
    period = 1;
  }
  int32_t timer_val = (int32_t)apu->ch2_freq_timer - (int32_t)cycles;
  while (timer_val <= 0) {
    apu->ch2_duty_pos = (uint8_t)((unsigned)apu->ch2_duty_pos + 1U) & 7U;
    timer_val += (int32_t)period;
  }
  apu->ch2_freq_timer = (uint16_t)timer_val;
}
