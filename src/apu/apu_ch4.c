#include <stdint.h>

#include "apu_private.h"

void apu_write_ch4(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF20:
      apu->nr41 = val;
      apu->ch4_len = val & (APU_CH4_LEN_MAX - 1);
      break;
    case 0xFF21:
      apu->nr42 = val;
      if (!(val & 0xF8)) apu->ch4_active = false;
      break;
    case 0xFF22:
      apu->nr43 = val;
      break;
    case 0xFF23: {
      bool old_len_enable = apu->ch4_len_enable;
      apu->ch4_len_enable = ((val >> 6) & 1) != 0;
      if (!old_len_enable && apu->ch4_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch4_len, APU_CH4_LEN_MAX))
          apu->ch4_active = false;
      }
      apu->nr44 = val & 0x7F;
      if (val & (1u << NR52_POWER_BIT)) {
        if (apu->ch4_len >= APU_CH4_LEN_MAX) {
          apu->ch4_len =
              (apu->ch4_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr42 & 0xF8) apu->ch4_active = true;
        apu->ch4_lfsr = APU_LFSR_INIT;
        apu->ch4_env_vol = (apu->nr42 >> 4) & 0x0F;
        uint8_t ep4 = apu->nr42 & 0x07;
        apu->ch4_env_timer = ep4 ? ep4 : 8;
        uint8_t r4 = apu->nr43 & 0x07;
        uint8_t s4 = (apu->nr43 >> 4) & 0x0F;
        apu->ch4_freq_timer = (uint32_t)(r4 == 0 ? 8u : (uint32_t)r4 * 16u)
                              << s4;
        if (apu->ch4_freq_timer == 0) apu->ch4_freq_timer = 1;
      }
      break;
    }
  }
}

void tick_ch4_freq(struct Apu* apu, uint8_t cycles) {
  uint8_t r4 = apu->nr43 & 0x07;
  uint8_t s4 = (apu->nr43 >> 4) & 0x0F;
  uint32_t period = (uint32_t)(r4 == 0 ? 8u : (uint32_t)r4 * 16u) << s4;
  if (period == 0) period = 1;
  int32_t t = (int32_t)apu->ch4_freq_timer - (int32_t)cycles;
  while (t <= 0) {
    uint8_t xb = (uint8_t)((apu->ch4_lfsr & 1u) ^ ((apu->ch4_lfsr >> 1) & 1u));
    apu->ch4_lfsr >>= 1;
    apu->ch4_lfsr |= (uint16_t)(xb << 14);
    if (apu->nr43 & 0x08) {
      apu->ch4_lfsr &= (uint16_t)~(1u << 6);
      apu->ch4_lfsr |= (uint16_t)(xb << 6);
    }
    t += (int32_t)period;
  }
  apu->ch4_freq_timer = (uint32_t)t;
}
