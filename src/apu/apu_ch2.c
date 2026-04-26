#include <stdint.h>

#include "apu_private.h"

void apu_write_ch2(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF16:
      apu->nr21 = val;
      apu->ch2_len = val & (APU_CH2_LEN_MAX - 1);
      break;
    case 0xFF17:
      apu->nr22 = val;
      if (!(val & 0xF8)) apu->ch2_active = false;
      break;
    case 0xFF18:
      apu->nr23 = val;
      break;
    case 0xFF19: {
      bool old_len_enable = apu->ch2_len_enable;
      apu->ch2_len_enable = ((val >> 6) & 1) != 0;
      if (!old_len_enable && apu->ch2_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch2_len, APU_CH2_LEN_MAX))
          apu->ch2_active = false;
      }
      apu->nr24 = val & 0x7F;
      if (val & (1u << NR52_POWER_BIT)) {
        if (apu->ch2_len >= APU_CH2_LEN_MAX) {
          apu->ch2_len =
              (apu->ch2_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr22 & 0xF8) apu->ch2_active = true;
        uint16_t freq2 = ((uint16_t)(apu->nr24 & 0x07) << 8) | apu->nr23;
        apu->ch2_freq_timer = (uint16_t)((2048u - freq2) * 4u);
        apu->ch2_env_vol = (apu->nr22 >> 4) & 0x0F;
        uint8_t ep2 = apu->nr22 & 0x07;
        apu->ch2_env_timer = ep2 ? ep2 : 8;
      }
      break;
    }
  }
}

void tick_ch2_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq = ((uint16_t)(apu->nr24 & 0x07) << 8) | apu->nr23;
  uint16_t period = (uint16_t)((2048u - freq) * 4u);
  if (period == 0) period = 1;
  int32_t t = (int32_t)apu->ch2_freq_timer - (int32_t)cycles;
  while (t <= 0) {
    apu->ch2_duty_pos = (apu->ch2_duty_pos + 1) & 7;
    t += (int32_t)period;
  }
  apu->ch2_freq_timer = (uint16_t)t;
}
