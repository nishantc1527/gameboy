#include <stdint.h>

#include "apu_private.h"

void apu_write_ch4(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF20:
      apu->nr41 = val;
      apu->ch4_len = (uint8_t)((unsigned)val & (APU_CH4_LEN_MAX - 1U));
      break;
    case 0xFF21:
      apu->nr42 = val;
      if (!((unsigned)val & 0xF8U)) {
        apu->ch4_active = false;
      }
      break;
    case 0xFF22:
      apu->nr43 = val;
      break;
    case 0xFF23: {
      bool old_len_enable = apu->ch4_len_enable;
      apu->ch4_len_enable = (((unsigned)val >> 6U) & 1U) != 0;
      if (!old_len_enable && apu->ch4_len_enable &&
          ((unsigned)apu->seq_step & 1U) == 0U) {
        if (clock_length_u8(&apu->ch4_len, APU_CH4_LEN_MAX)) {
          apu->ch4_active = false;
        }
      }
      apu->nr44 = (uint8_t)((unsigned)val & 0x7FU);
      if ((unsigned)val & (1U << NR52_POWER_BIT)) {
        if (apu->ch4_len >= APU_CH4_LEN_MAX) {
          apu->ch4_len =
              (apu->ch4_len_enable && ((unsigned)apu->seq_step & 1U) == 0U) ? 1
                                                                            : 0;
        }
        if ((unsigned)apu->nr42 & 0xF8U) {
          apu->ch4_active = true;
        }
        apu->ch4_lfsr = APU_LFSR_INIT;
        apu->ch4_env_vol = (uint8_t)(((unsigned)apu->nr42 >> 4U) & 0x0FU);
        uint8_t ep4 = (uint8_t)((unsigned)apu->nr42 & 0x07U);
        apu->ch4_env_timer = ep4 ? ep4 : 8;
        uint8_t div4 = (uint8_t)((unsigned)apu->nr43 & 0x07U);
        uint8_t shift4 = (uint8_t)(((unsigned)apu->nr43 >> 4U) & 0x0FU);
        apu->ch4_freq_timer = (uint32_t)(div4 == 0 ? 8U : (uint32_t)div4 * 16U)
                              << shift4;
        if (apu->ch4_freq_timer == 0) {
          apu->ch4_freq_timer = 1;
        }
      }
      break;
    }
    default:
      break;
  }
}

void tick_ch4_freq(struct Apu* apu, uint8_t cycles) {
  uint8_t div4 = (uint8_t)((unsigned)apu->nr43 & 0x07U);
  uint8_t shift4 = (uint8_t)(((unsigned)apu->nr43 >> 4U) & 0x0FU);
  uint32_t period = (uint32_t)(div4 == 0 ? 8U : (uint32_t)div4 * 16U) << shift4;
  if (period == 0) {
    period = 1;
  }
  int32_t timer_val = (int32_t)apu->ch4_freq_timer - (int32_t)cycles;
  while (timer_val <= 0) {
    uint8_t xor_bit = (uint8_t)(((unsigned)apu->ch4_lfsr & 1U) ^
                                (((unsigned)apu->ch4_lfsr >> 1U) & 1U));
    apu->ch4_lfsr >>= 1;
    apu->ch4_lfsr |= (uint16_t)(xor_bit << 14U);
    if ((unsigned)apu->nr43 & 0x08U) {
      apu->ch4_lfsr &= (uint16_t)~(1U << 6U);
      apu->ch4_lfsr |= (uint16_t)(xor_bit << 6U);
    }
    timer_val += (int32_t)period;
  }
  apu->ch4_freq_timer = (uint32_t)timer_val;
}
