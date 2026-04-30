#include <stdint.h>

#include "apu_private.h"
#include "gbemu/apu.h"

static const uint8_t DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

void apu_mix_samples(struct Apu* apu, uint8_t cycles) {
  apu->sample_acc += (uint32_t)cycles * 375U;
  const uint32_t cps = 32768U;
  uint8_t dac1_on = ((unsigned)apu->nr12 & 0xF8U) != 0;
  uint8_t dac2_on = ((unsigned)apu->nr22 & 0xF8U) != 0;
  uint8_t dac3_on = ((unsigned)apu->nr30 & 0x80U) != 0;
  uint8_t dac4_on = ((unsigned)apu->nr42 & 0xF8U) != 0;
  uint8_t any_dac_on = (uint8_t)((unsigned)dac1_on | (unsigned)dac2_on |
                                 (unsigned)dac3_on | (unsigned)dac4_on);
  while (apu->sample_acc >= cps && apu->sample_count < APU_BUF_SIZE) {
    apu->sample_acc -= cps;
    float left = 0.0F;
    float right = 0.0F;
    if (dac1_on) {
      uint8_t duty1 = (uint8_t)(((unsigned)apu->nr11 >> 6U) & 3U);
      uint8_t dig = 0U;
      if (apu->ch1_active) {
        dig = DUTY_TABLE[duty1][apu->ch1_duty_pos] ? apu->ch1_env_vol : 0U;
      }
      float sample = 1.0F - (2.0F * (float)dig / 15.0F);
      if ((unsigned)apu->nr51 & 0x10U) {
        left += sample;
      }
      if ((unsigned)apu->nr51 & 0x01U) {
        right += sample;
      }
    }
    if (dac2_on) {
      uint8_t duty2 = (uint8_t)(((unsigned)apu->nr21 >> 6U) & 3U);
      uint8_t dig = 0U;
      if (apu->ch2_active) {
        dig = DUTY_TABLE[duty2][apu->ch2_duty_pos] ? apu->ch2_env_vol : 0U;
      }
      float sample = 1.0F - (2.0F * (float)dig / 15.0F);
      if ((unsigned)apu->nr51 & 0x20U) {
        left += sample;
      }
      if ((unsigned)apu->nr51 & 0x02U) {
        right += sample;
      }
    }
    if (dac3_on) {
      uint8_t vol_code = (uint8_t)(((unsigned)apu->nr32 >> 5U) & 3U);
      uint8_t shift = vol_code == 0 ? 4 : vol_code - 1;
      uint8_t dig = 0;
      if (apu->ch3_active) {
        uint8_t wave_byte = apu->wave_ram[(unsigned)apu->ch3_pos / 2U];
        uint8_t nibble = ((unsigned)apu->ch3_pos & 1U)
                             ? ((unsigned)wave_byte & 0x0FU)
                             : ((unsigned)wave_byte >> 4U);
        dig = (uint8_t)((unsigned)nibble >> shift);
      }
      float sample = 1.0F - (2.0F * (float)dig / 15.0F);
      if ((unsigned)apu->nr51 & 0x40U) {
        left += sample;
      }
      if ((unsigned)apu->nr51 & 0x04U) {
        right += sample;
      }
    }
    if (dac4_on) {
      uint8_t dig = 0U;
      if (apu->ch4_active) {
        dig = (~(unsigned)apu->ch4_lfsr & 1U) ? apu->ch4_env_vol : 0U;
      }
      float sample = 1.0F - (2.0F * (float)dig / 15.0F);
      if ((unsigned)apu->nr51 & 0x80U) {
        left += sample;
      }
      if ((unsigned)apu->nr51 & 0x08U) {
        right += sample;
      }
    }
    float lvol = (float)((((unsigned)apu->nr50 >> 4U) & 7U) + 1U) / 8.0F;
    float rvol = (float)(((unsigned)apu->nr50 & 7U) + 1U) / 8.0F;
    left *= lvol;
    right *= rvol;
    if (any_dac_on) {
      float out_L = left - (float)apu->capacitor_L;
      apu->capacitor_L = (double)(left - (out_L * HPF_CHARGE));
      float out_R = right - (float)apu->capacitor_R;
      apu->capacitor_R = (double)(right - (out_R * HPF_CHARGE));
      left = out_L;
      right = out_R;
    } else {
      left = 0.0F;
      right = 0.0F;
    }
    apu->sample_buf[apu->sample_count][0] = left / 4.0F;
    apu->sample_buf[apu->sample_count][1] = right / 4.0F;
    apu->sample_count++;
  }
}
