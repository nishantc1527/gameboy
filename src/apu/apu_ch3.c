#include <stdint.h>

#include "apu_private.h"

void apu_write_ch3(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF1A:
      apu->nr30 = val;
      if (!(val & (1u << NR52_POWER_BIT))) apu->ch3_active = false;
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
      apu->ch3_len_enable = ((val >> 6) & 1) != 0;
      if (!old_len_enable && apu->ch3_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u16(&apu->ch3_len, APU_CH3_LEN_MAX))
          apu->ch3_active = false;
      }
      apu->nr34 = val & 0x7F;
      if (val & (1u << NR52_POWER_BIT)) {
        if (apu->ch3_len >= APU_CH3_LEN_MAX) {
          apu->ch3_len =
              (apu->ch3_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr30 & (1u << NR52_POWER_BIT)) apu->ch3_active = true;
        uint16_t freq3 = ((uint16_t)(apu->nr34 & 0x07) << 8) | apu->nr33;
        apu->ch3_freq_timer = (uint16_t)((2048u - freq3) * 2u);
        apu->ch3_pos = 0;
      }
      break;
    }
  }
}

void tick_ch3_freq(struct Apu* apu, uint8_t cycles) {
  uint16_t freq = ((uint16_t)(apu->nr34 & 0x07) << 8) | apu->nr33;
  uint16_t period = (uint16_t)((2048u - freq) * 2u);
  if (period == 0) period = 1;
  int32_t t = (int32_t)apu->ch3_freq_timer - (int32_t)cycles;
  while (t <= 0) {
    apu->ch3_pos = (apu->ch3_pos + 1) & 31;
    t += (int32_t)period;
  }
  apu->ch3_freq_timer = (uint16_t)t;
}
