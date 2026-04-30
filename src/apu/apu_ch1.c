#include <stdint.h>

#include "apu_private.h"
#include "gbemu/util.h"

static uint16_t sweep_calc(struct Apu* apu, uint8_t* overflow) {
  uint8_t shift = (uint8_t)((unsigned)apu->nr10 & 0x07U);
  uint8_t negate = get_bit(apu->nr10, 3);
  uint16_t delta = (uint16_t)((unsigned)apu->sweep_freq >> shift);
  uint16_t new_freq = 0;
  if (negate) {
    apu->sweep_neg_used = true;
    new_freq = apu->sweep_freq - delta;
  } else {
    new_freq = apu->sweep_freq + delta;
  }
  *overflow = (new_freq > 0x7FF) ? 1 : 0;
  return new_freq;
}

void apu_clock_sweep(struct Apu* apu) {
  if (!apu->sweep_enable) {
    return;
  }
  if (apu->sweep_timer > 0) {
    apu->sweep_timer--;
  }
  if (apu->sweep_timer == 0) {
    uint8_t pace = (uint8_t)(((unsigned)apu->nr10 >> 4U) & 0x07U);
    apu->sweep_timer = (pace != 0) ? pace : 8;
    if (pace != 0) {
      uint8_t overflow = 0;
      uint16_t new_freq = sweep_calc(apu, &overflow);
      if (overflow) {
        apu->ch1_active = false;
      } else {
        uint8_t shift = (uint8_t)((unsigned)apu->nr10 & 0x07U);
        if (shift != 0) {
          apu->sweep_freq = new_freq;
          apu->nr13 = (uint8_t)((unsigned)new_freq & 0xFFU);
          apu->nr14 = (uint8_t)(((unsigned)apu->nr14 & 0xF8U) |
                                (((unsigned)new_freq >> 8U) & 0x07U));
          overflow = 0;
          sweep_calc(apu, &overflow);
          if (overflow) {
            apu->ch1_active = false;
          }
        }
      }
    }
  }
  if (apu->sweep_neg_used && !get_bit(apu->nr10, 3)) {
    apu->ch1_active = false;
    apu->sweep_neg_used = false;
  }
}

void apu_write_ch1(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF10:
      apu->nr10 = val;
      if (apu->sweep_neg_used && !get_bit(val, 3)) {
        apu->ch1_active = false;
        apu->sweep_neg_used = false;
      }
      break;
    case 0xFF11:
      apu->nr11 = val;
      apu->ch1_len = (uint8_t)((unsigned)val & (APU_CH1_LEN_MAX - 1U));
      break;
    case 0xFF12:
      apu->nr12 = val;
      if (!((unsigned)val & 0xF8U)) {
        apu->ch1_active = false;
      }
      break;
    case 0xFF13:
      apu->nr13 = val;
      break;
    case 0xFF14: {
      bool old_len_enable = apu->ch1_len_enable;
      apu->ch1_len_enable = (((unsigned)val >> 6U) & 1U) != 0;
      if (!old_len_enable && apu->ch1_len_enable &&
          ((unsigned)apu->seq_step & 1U) == 0U) {
        if (clock_length_u8(&apu->ch1_len, APU_CH1_LEN_MAX)) {
          apu->ch1_active = false;
        }
      }
      apu->nr14 = (uint8_t)((unsigned)val & 0x7FU);
      if ((unsigned)val & (1U << NR52_POWER_BIT)) {
        if (apu->ch1_len >= APU_CH1_LEN_MAX) {
          apu->ch1_len =
              (apu->ch1_len_enable && ((unsigned)apu->seq_step & 1U) == 0U) ? 1
                                                                            : 0;
        }
        if ((unsigned)apu->nr12 & 0xF8U) {
          apu->ch1_active = true;
          uint8_t pace = (uint8_t)(((unsigned)apu->nr10 >> 4U) & 0x07U);
          uint8_t shift = (uint8_t)((unsigned)apu->nr10 & 0x07U);
          apu->sweep_freq = (uint16_t)(((unsigned)(apu->nr14 & 0x07U) << 8U) |
                                       (unsigned)apu->nr13);
          apu->sweep_timer = (pace != 0) ? pace : 8;
          apu->sweep_enable = ((pace != 0 || shift != 0) != 0);
          apu->sweep_neg_used = false;
          if (shift != 0) {
            uint8_t overflow = 0;
            sweep_calc(apu, &overflow);
            if (overflow) {
              apu->ch1_active = false;
            }
          }
        }
        uint16_t freq1 = (uint16_t)(((unsigned)(apu->nr14 & 0x07U) << 8U) |
                                    (unsigned)apu->nr13);
        apu->ch1_freq_timer = (uint16_t)((2048U - freq1) * 4U);
        apu->ch1_env_vol = (uint8_t)(((unsigned)apu->nr12 >> 4U) & 0x0FU);
        uint8_t ep1 = (uint8_t)((unsigned)apu->nr12 & 0x07U);
        apu->ch1_env_timer = ep1 ? ep1 : 8;
      }
      break;
    }
    default:
      break;
  }
}

void tick_ch1_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq =
      (uint16_t)(((unsigned)(apu->nr14 & 0x07U) << 8U) | (unsigned)apu->nr13);
  uint16_t period = (uint16_t)((2048U - freq) * 4U);
  if (period == 0) {
    period = 1;
  }
  int32_t timer_val = (int32_t)apu->ch1_freq_timer - (int32_t)cycles;
  while (timer_val <= 0) {
    apu->ch1_duty_pos = (uint8_t)((unsigned)apu->ch1_duty_pos + 1U) & 7U;
    timer_val += (int32_t)period;
  }
  apu->ch1_freq_timer = (uint16_t)timer_val;
}
