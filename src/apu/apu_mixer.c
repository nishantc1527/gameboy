#include <stdint.h>

#include "apu_private.h"

static const uint8_t DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

void apu_mix_samples(struct Apu* apu, uint8_t cycles) {
  apu->sample_acc += (uint32_t)cycles * 375u;
  const uint32_t cps = 32768u;
  uint8_t dac1_on = (apu->nr12 & 0xF8) != 0;
  uint8_t dac2_on = (apu->nr22 & 0xF8) != 0;
  uint8_t dac3_on = (apu->nr30 & 0x80) != 0;
  uint8_t dac4_on = (apu->nr42 & 0xF8) != 0;
  uint8_t any_dac_on = dac1_on | dac2_on | dac3_on | dac4_on;
  while (apu->sample_acc >= cps && apu->sample_count < APU_BUF_SIZE) {
    apu->sample_acc -= cps;
    float L = 0.0f, R = 0.0f;
    if (dac1_on) {
      uint8_t duty1 = (apu->nr11 >> 6) & 3;
      uint8_t dig =
          apu->ch1_active
              ? (DUTY_TABLE[duty1][apu->ch1_duty_pos] ? apu->ch1_env_vol : 0u)
              : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (apu->nr51 & 0x10) L += s;
      if (apu->nr51 & 0x01) R += s;
    }
    if (dac2_on) {
      uint8_t duty2 = (apu->nr21 >> 6) & 3;
      uint8_t dig =
          apu->ch2_active
              ? (DUTY_TABLE[duty2][apu->ch2_duty_pos] ? apu->ch2_env_vol : 0u)
              : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (apu->nr51 & 0x20) L += s;
      if (apu->nr51 & 0x02) R += s;
    }
    if (dac3_on) {
      uint8_t vol_code = (apu->nr32 >> 5) & 3;
      uint8_t shift = vol_code == 0 ? 4 : vol_code - 1;
      uint8_t dig = 0;
      if (apu->ch3_active) {
        uint8_t wave_byte = apu->wave_ram[apu->ch3_pos / 2];
        uint8_t nibble =
            (apu->ch3_pos & 1) ? (wave_byte & 0x0Fu) : (wave_byte >> 4);
        dig = nibble >> shift;
      }
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (apu->nr51 & 0x40) L += s;
      if (apu->nr51 & 0x04) R += s;
    }
    if (dac4_on) {
      uint8_t dig = apu->ch4_active
                        ? ((~apu->ch4_lfsr & 1u) ? apu->ch4_env_vol : 0u)
                        : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (apu->nr51 & 0x80) L += s;
      if (apu->nr51 & 0x08) R += s;
    }
    float lvol = (float)(((apu->nr50 >> 4) & 7) + 1) / 8.0f;
    float rvol = (float)((apu->nr50 & 7) + 1) / 8.0f;
    L *= lvol;
    R *= rvol;
    if (any_dac_on) {
      float out_L = L - (float)apu->capacitor_L;
      apu->capacitor_L = (double)(L - out_L * HPF_CHARGE);
      float out_R = R - (float)apu->capacitor_R;
      apu->capacitor_R = (double)(R - out_R * HPF_CHARGE);
      L = out_L;
      R = out_R;
    } else {
      L = 0.0f;
      R = 0.0f;
    }
    apu->sample_buf[apu->sample_count][0] = L / 4.0f;
    apu->sample_buf[apu->sample_count][1] = R / 4.0f;
    apu->sample_count++;
  }
}
