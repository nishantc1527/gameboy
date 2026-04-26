#include <stdint.h>

#include "apu_private.h"

static uint16_t sweep_calc(struct Apu* apu, uint8_t* overflow) {
  uint8_t shift = apu->nr10 & 0x07;
  uint8_t negate = get_bit(apu->nr10, 3);
  uint16_t delta = apu->sweep_freq >> shift;
  uint16_t new_freq;
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
  if (!apu->sweep_enable) return;
  if (apu->sweep_timer > 0) apu->sweep_timer--;
  if (apu->sweep_timer == 0) {
    uint8_t pace = (apu->nr10 >> 4) & 0x07;
    apu->sweep_timer = (pace != 0) ? pace : 8;
    if (pace != 0) {
      uint8_t overflow = 0;
      uint16_t new_freq = sweep_calc(apu, &overflow);
      if (overflow) {
        apu->ch1_active = false;
      } else {
        uint8_t shift = apu->nr10 & 0x07;
        if (shift != 0) {
          apu->sweep_freq = new_freq;
          apu->nr13 = (uint8_t)(new_freq & 0xFF);
          apu->nr14 = (apu->nr14 & 0xF8) | ((new_freq >> 8) & 0x07);
          overflow = 0;
          sweep_calc(apu, &overflow);
          if (overflow) apu->ch1_active = false;
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
      apu->ch1_len = val & (APU_CH1_LEN_MAX - 1);
      break;
    case 0xFF12:
      apu->nr12 = val;
      if (!(val & 0xF8)) apu->ch1_active = false;
      break;
    case 0xFF13:
      apu->nr13 = val;
      break;
    case 0xFF14: {
      bool old_len_enable = apu->ch1_len_enable;
      apu->ch1_len_enable = ((val >> 6) & 1) != 0;
      if (!old_len_enable && apu->ch1_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch1_len, APU_CH1_LEN_MAX))
          apu->ch1_active = false;
      }
      apu->nr14 = val & 0x7F;
      if (val & (1u << NR52_POWER_BIT)) {
        if (apu->ch1_len >= APU_CH1_LEN_MAX) {
          apu->ch1_len =
              (apu->ch1_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr12 & 0xF8) {
          apu->ch1_active = true;
          uint8_t pace = (apu->nr10 >> 4) & 0x07;
          uint8_t shift = apu->nr10 & 0x07;
          apu->sweep_freq = ((uint16_t)(apu->nr14 & 0x07) << 8) | apu->nr13;
          apu->sweep_timer = (pace != 0) ? pace : 8;
          apu->sweep_enable = (pace != 0 || shift != 0);
          apu->sweep_neg_used = false;
          if (shift != 0) {
            uint8_t overflow = 0;
            sweep_calc(apu, &overflow);
            if (overflow) apu->ch1_active = false;
          }
        }
        uint16_t freq1 = ((uint16_t)(apu->nr14 & 0x07) << 8) | apu->nr13;
        apu->ch1_freq_timer = (uint16_t)((2048u - freq1) * 4u);
        apu->ch1_env_vol = (apu->nr12 >> 4) & 0x0F;
        uint8_t ep1 = apu->nr12 & 0x07;
        apu->ch1_env_timer = ep1 ? ep1 : 8;
      }
      break;
    }
  }
}

void tick_ch1_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq = ((uint16_t)(apu->nr14 & 0x07) << 8) | apu->nr13;
  uint16_t period = (uint16_t)((2048u - freq) * 4u);
  if (period == 0) period = 1;
  int32_t t = (int32_t)apu->ch1_freq_timer - (int32_t)cycles;
  while (t <= 0) {
    apu->ch1_duty_pos = (apu->ch1_duty_pos + 1) & 7;
    t += (int32_t)period;
  }
  apu->ch1_freq_timer = (uint16_t)t;
}
