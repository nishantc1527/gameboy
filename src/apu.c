#include "gbemu/apu.h"

#include "gbemu/mmu.h"
#include "gbemu/util.h"

#define HPF_CHARGE 0.99634f

static const uint8_t DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

struct Apu* init_apu() {
  struct Apu* apu = malloc(sizeof(struct Apu));
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len = apu->ch2_len = apu->ch4_len = 0;
  apu->ch3_len = 0;
  apu->ch1_len_enable = apu->ch2_len_enable = apu->ch3_len_enable =
      apu->ch4_len_enable = 0;
  apu->div_apu = 0;
  apu->length_clock = 0;
  apu->sweep_clock = 0;
  apu->apu_on = 0;
  apu->sweep_freq = 0;
  apu->sweep_timer = 0;
  apu->sweep_enable = 0;
  apu->sweep_neg_used = 0;
  apu->ch_len_dirty = 0;
  apu->ch1_freq_timer = 0;
  apu->ch2_freq_timer = 0;
  apu->ch3_freq_timer = 0;
  apu->ch4_freq_timer = 0;
  apu->ch1_duty_pos = 0;
  apu->ch2_duty_pos = 0;
  apu->ch3_pos = 0;
  apu->ch4_lfsr = 0x7FFF;
  apu->ch1_env_vol = 0;
  apu->ch2_env_vol = 0;
  apu->ch4_env_vol = 0;
  apu->ch1_env_timer = 0;
  apu->ch2_env_timer = 0;
  apu->ch4_env_timer = 0;
  apu->envelope_clock = 0;
  apu->sample_count = 0;
  apu->sample_acc = 0;
  apu->capacitor_L = 0.0;
  apu->capacitor_R = 0.0;
  return apu;
}

static uint16_t sweep_calc(struct Apu* apu, uint8_t nr10, uint8_t* overflow) {
  uint8_t shift = nr10 & 0x07;
  uint8_t negate = get_bit(nr10, 3);
  uint16_t delta = apu->sweep_freq >> shift;
  uint16_t new_freq;
  if (negate) {
    apu->sweep_neg_used = 1;
    new_freq = apu->sweep_freq - delta;
  } else {
    new_freq = apu->sweep_freq + delta;
  }
  *overflow = (new_freq > 0x7FF) ? 1 : 0;
  return new_freq;
}

static uint8_t clock_length_u8(uint8_t* len, uint8_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}
static uint8_t clock_length_u16(uint16_t* len, uint16_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}

#define TICK_FREQ16(timer, cycles, period, advance)    \
  do {                                                 \
    int32_t _t = (int32_t)(timer) - (int32_t)(cycles); \
    while (_t <= 0) {                                  \
      {                                                \
        advance;                                       \
      }                                                \
      _t += (int32_t)(period);                         \
    }                                                  \
    (timer) = (uint16_t)_t;                            \
  } while (0)

#define TICK_FREQ32(timer, cycles, period, advance)    \
  do {                                                 \
    int32_t _t = (int32_t)(timer) - (int32_t)(cycles); \
    while (_t <= 0) {                                  \
      {                                                \
        advance;                                       \
      }                                                \
      _t += (int32_t)(period);                         \
    }                                                  \
    (timer) = (uint32_t)_t;                            \
  } while (0)

void upd_apu(struct Apu* apu, struct Mmu* mmu, uint8_t cycles) {
  uint8_t nr52 = mmu_r_mem(mmu, NR52);
  uint8_t curr_apu_on = get_bit(nr52, 7);

  if (apu->apu_on && !curr_apu_on) {
    reset_apu(apu, mmu);
    apu->apu_on = 0;
    apu->length_clock = 0;
    apu->sweep_clock = 0;
    return;
  }
  apu->apu_on = curr_apu_on;

  uint8_t len_dirty = mmu_take_len_dirty(mmu);

  if (!curr_apu_on) {
    if (len_dirty & 0x01) {
      uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
      apu->ch1_len = nr11 & 0x3F;
    }
    if (len_dirty & 0x02) {
      uint8_t nr21 = mmu_r_mem_raw(mmu, NR21);
      apu->ch2_len = nr21 & 0x3F;
    }
    if (len_dirty & 0x04) {
      uint8_t nr31 = mmu_r_mem_raw(mmu, NR31);
      apu->ch3_len = nr31;
    }
    if (len_dirty & 0x08) {
      uint8_t nr41 = mmu_r_mem_raw(mmu, NR41);
      apu->ch4_len = nr41 & 0x3F;
    }
    apu->length_clock = 0;
    apu->sweep_clock = 0;
    apu->sample_acc += (uint32_t)cycles * 375u;
    const uint32_t cps = 32768u;
    while (apu->sample_acc >= cps && apu->sample_count < APU_BUF_SIZE) {
      apu->sample_acc -= cps;
      apu->sample_buf[apu->sample_count][0] = 0.0f;
      apu->sample_buf[apu->sample_count][1] = 0.0f;
      apu->sample_count++;
    }
    return;
  }

  uint8_t nr10 = mmu_r_mem_raw(mmu, NR10);
  uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
  uint8_t nr12 = mmu_r_mem(mmu, NR12);
  uint8_t nr13 = mmu_r_mem_raw(mmu, NR13);
  uint8_t nr14 = mmu_r_mem_raw(mmu, NR14);
  uint8_t curr_ch1_len_enable = get_bit(nr14, 6);
  uint8_t ch1_triggered = get_bit(nr14, 7);
  if ((len_dirty & 0x01) || (nr11 & 0x3F) != 0x3F) apu->ch1_len = nr11 & 0x3F;
  if (!apu->ch1_len_enable && curr_ch1_len_enable && (apu->div_apu & 1) == 0) {
    if (clock_length_u8(&apu->ch1_len, 0x40)) apu->ch1_enable = 0;
  }
  if (ch1_triggered) {
    if (apu->ch1_len >= 0x40) {
      apu->ch1_len = (curr_ch1_len_enable && (apu->div_apu & 1) == 0) ? 1 : 0;
    }
    if (nr12 & 0xF8) {
      apu->ch1_enable = 1;
      uint8_t pace = (nr10 >> 4) & 0x07;
      uint8_t shift = nr10 & 0x07;
      apu->sweep_freq = ((uint16_t)(nr14 & 0x07) << 8) | nr13;
      apu->sweep_timer = (pace != 0) ? pace : 8;
      apu->sweep_enable = (pace != 0 || shift != 0) ? 1 : 0;
      apu->sweep_neg_used = 0;
      if (shift != 0) {
        uint8_t overflow = 0;
        sweep_calc(apu, nr10, &overflow);
        if (overflow) apu->ch1_enable = 0;
      }
    }
    apu->ch1_freq_timer =
        (uint16_t)((2048u - (((uint16_t)(nr14 & 0x07) << 8) | nr13)) * 4u);
    apu->ch1_env_vol = (nr12 >> 4) & 0x0F;
    uint8_t ep1 = nr12 & 0x07;
    apu->ch1_env_timer = ep1 ? ep1 : 8;
  }
  apu->ch1_len_enable = curr_ch1_len_enable;
  if (apu->length_clock && apu->ch1_len_enable) {
    if (clock_length_u8(&apu->ch1_len, 0x40)) apu->ch1_enable = 0;
  }
  if (apu->sweep_clock && apu->sweep_enable) {
    if (apu->sweep_timer > 0) apu->sweep_timer--;
    if (apu->sweep_timer == 0) {
      uint8_t pace = (nr10 >> 4) & 0x07;
      apu->sweep_timer = (pace != 0) ? pace : 8;
      if (pace != 0) {
        uint8_t overflow = 0;
        uint16_t new_freq = sweep_calc(apu, nr10, &overflow);
        if (overflow) {
          apu->ch1_enable = 0;
        } else {
          uint8_t shift = nr10 & 0x07;
          if (shift != 0) {
            apu->sweep_freq = new_freq;
            mmu_w_mem_raw(mmu, NR13, (uint8_t)(new_freq & 0xFF));
            uint8_t nr14u = (nr14 & 0xF8) | ((new_freq >> 8) & 0x07);
            mmu_w_mem_raw(mmu, NR14, nr14u);
            overflow = 0;
            sweep_calc(apu, nr10, &overflow);
            if (overflow) apu->ch1_enable = 0;
          }
        }
      }
    }
  }
  if (apu->sweep_neg_used && !get_bit(nr10, 3)) {
    apu->ch1_enable = 0;
    apu->sweep_neg_used = 0;
  }
  clear_bit(&nr14, 7);
  mmu_w_mem_raw(mmu, NR14, nr14);
  nr11 |= 0x3F;
  mmu_w_mem_raw(mmu, NR11, nr11);
  if (!(nr12 & 0xF8)) apu->ch1_enable = 0;
  if (apu->ch1_freq_timer > 0) {
    uint16_t freq1 = ((uint16_t)(mmu_r_mem_raw(mmu, NR14) & 0x07) << 8) |
                     mmu_r_mem_raw(mmu, NR13);
    uint16_t period1 = (uint16_t)((2048u - freq1) * 4u);
    if (period1 == 0) period1 = 1;
    TICK_FREQ16(apu->ch1_freq_timer, cycles, period1,
                apu->ch1_duty_pos = (apu->ch1_duty_pos + 1) & 7);
  }
  uint8_t nr21 = mmu_r_mem_raw(mmu, NR21);
  uint8_t nr22 = mmu_r_mem(mmu, NR22);
  uint8_t nr23 = mmu_r_mem_raw(mmu, NR23);
  uint8_t nr24 = mmu_r_mem_raw(mmu, NR24);
  uint8_t curr_ch2_len_enable = get_bit(nr24, 6);
  uint8_t ch2_triggered = get_bit(nr24, 7);
  if ((len_dirty & 0x02) || (nr21 & 0x3F) != 0x3F) apu->ch2_len = nr21 & 0x3F;
  if (!apu->ch2_len_enable && curr_ch2_len_enable && (apu->div_apu & 1) == 0) {
    if (clock_length_u8(&apu->ch2_len, 0x40)) apu->ch2_enable = 0;
  }
  if (ch2_triggered) {
    if (apu->ch2_len >= 0x40) {
      apu->ch2_len = (curr_ch2_len_enable && (apu->div_apu & 1) == 0) ? 1 : 0;
    }
    if (nr22 & 0xF8) apu->ch2_enable = 1;
    apu->ch2_freq_timer =
        (uint16_t)((2048u - (((uint16_t)(nr24 & 0x07) << 8) | nr23)) * 4u);
    apu->ch2_env_vol = (nr22 >> 4) & 0x0F;
    uint8_t ep2 = nr22 & 0x07;
    apu->ch2_env_timer = ep2 ? ep2 : 8;
  }
  apu->ch2_len_enable = curr_ch2_len_enable;
  if (apu->length_clock && apu->ch2_len_enable) {
    if (clock_length_u8(&apu->ch2_len, 0x40)) apu->ch2_enable = 0;
  }
  clear_bit(&nr24, 7);
  mmu_w_mem_raw(mmu, NR24, nr24);
  nr21 |= 0x3F;
  mmu_w_mem_raw(mmu, NR21, nr21);
  if (!(nr22 & 0xF8)) apu->ch2_enable = 0;
  if (apu->ch2_freq_timer > 0) {
    uint16_t freq2 = ((uint16_t)(mmu_r_mem_raw(mmu, NR24) & 0x07) << 8) | nr23;
    uint16_t period2 = (uint16_t)((2048u - freq2) * 4u);
    if (period2 == 0) period2 = 1;
    TICK_FREQ16(apu->ch2_freq_timer, cycles, period2,
                apu->ch2_duty_pos = (apu->ch2_duty_pos + 1) & 7);
  }
  uint8_t nr30 = mmu_r_mem_raw(mmu, NR30);
  uint8_t nr31 = mmu_r_mem_raw(mmu, NR31);
  uint8_t nr33 = mmu_r_mem_raw(mmu, NR33);
  uint8_t nr34 = mmu_r_mem_raw(mmu, NR34);
  uint8_t curr_ch3_len_enable = get_bit(nr34, 6);
  uint8_t ch3_triggered = get_bit(nr34, 7);
  if ((len_dirty & 0x04) || nr31 != 0xFF) apu->ch3_len = nr31;
  if (!apu->ch3_len_enable && curr_ch3_len_enable && (apu->div_apu & 1) == 0) {
    if (clock_length_u16(&apu->ch3_len, 0x100)) apu->ch3_enable = 0;
  }
  if (ch3_triggered) {
    if (apu->ch3_len >= 0x100) {
      apu->ch3_len = (curr_ch3_len_enable && (apu->div_apu & 1) == 0) ? 1 : 0;
    }
    if (nr30 & 0x80) apu->ch3_enable = 1;
    apu->ch3_freq_timer =
        (uint16_t)((2048u - (((uint16_t)(nr34 & 0x07) << 8) | nr33)) * 2u);
    apu->ch3_pos = 0;
  }
  apu->ch3_len_enable = curr_ch3_len_enable;
  if (apu->length_clock && apu->ch3_len_enable) {
    if (clock_length_u16(&apu->ch3_len, 0x100)) apu->ch3_enable = 0;
  }
  clear_bit(&nr34, 7);
  mmu_w_mem_raw(mmu, NR34, nr34);
  mmu_w_mem_raw(mmu, NR31, 0xFF);
  if (!(nr30 & 0x80)) apu->ch3_enable = 0;
  if (apu->ch3_freq_timer > 0) {
    uint16_t freq3 = ((uint16_t)(mmu_r_mem_raw(mmu, NR34) & 0x07) << 8) | nr33;
    uint16_t period3 = (uint16_t)((2048u - freq3) * 2u);
    if (period3 == 0) period3 = 1;
    TICK_FREQ16(apu->ch3_freq_timer, cycles, period3,
                apu->ch3_pos = (apu->ch3_pos + 1) & 31);
  }
  uint8_t nr41 = mmu_r_mem_raw(mmu, NR41);
  uint8_t nr42 = mmu_r_mem(mmu, NR42);
  uint8_t nr43 = mmu_r_mem_raw(mmu, NR43);
  uint8_t nr44 = mmu_r_mem_raw(mmu, NR44);
  uint8_t curr_ch4_len_enable = get_bit(nr44, 6);
  uint8_t ch4_triggered = get_bit(nr44, 7);
  if ((len_dirty & 0x08) || (nr41 & 0x3F) != 0x3F) apu->ch4_len = nr41 & 0x3F;
  if (!apu->ch4_len_enable && curr_ch4_len_enable && (apu->div_apu & 1) == 0) {
    if (clock_length_u8(&apu->ch4_len, 0x40)) apu->ch4_enable = 0;
  }
  if (ch4_triggered) {
    if (apu->ch4_len >= 0x40) {
      apu->ch4_len = (curr_ch4_len_enable && (apu->div_apu & 1) == 0) ? 1 : 0;
    }
    if (nr42 & 0xF8) apu->ch4_enable = 1;
    apu->ch4_lfsr = 0x7FFF;
    apu->ch4_env_vol = (nr42 >> 4) & 0x0F;
    uint8_t ep4 = nr42 & 0x07;
    apu->ch4_env_timer = ep4 ? ep4 : 8;
    uint8_t r4 = nr43 & 0x07;
    uint8_t s4 = (nr43 >> 4) & 0x0F;
    apu->ch4_freq_timer = (uint32_t)(r4 == 0 ? 8u : (uint32_t)r4 * 16u) << s4;
    if (apu->ch4_freq_timer == 0) apu->ch4_freq_timer = 1;
  }
  apu->ch4_len_enable = curr_ch4_len_enable;
  if (apu->length_clock && apu->ch4_len_enable) {
    if (clock_length_u8(&apu->ch4_len, 0x40)) apu->ch4_enable = 0;
  }
  clear_bit(&nr44, 7);
  mmu_w_mem_raw(mmu, NR44, nr44);
  nr41 |= 0x3F;
  mmu_w_mem_raw(mmu, NR41, nr41);
  if (!(nr42 & 0xF8)) apu->ch4_enable = 0;
  if (apu->ch4_freq_timer > 0) {
    uint8_t r4 = nr43 & 0x07;
    uint8_t s4 = (nr43 >> 4) & 0x0F;
    uint32_t period4 = (uint32_t)(r4 == 0 ? 8u : (uint32_t)r4 * 16u) << s4;
    if (period4 == 0) period4 = 1;
    TICK_FREQ32(apu->ch4_freq_timer, cycles, period4, {
      uint8_t xb =
          (uint8_t)((apu->ch4_lfsr & 1u) ^ ((apu->ch4_lfsr >> 1) & 1u));
      apu->ch4_lfsr >>= 1;
      apu->ch4_lfsr |= (uint16_t)(xb << 14);
      if (nr43 & 0x08) {
        apu->ch4_lfsr &= (uint16_t)~(1u << 6);
        apu->ch4_lfsr |= (uint16_t)(xb << 6);
      }
    });
  }
  if (apu->envelope_clock) {
    uint8_t ep;
    ep = nr12 & 0x07;
    if (ep != 0) {
      if (apu->ch1_env_timer > 0) apu->ch1_env_timer--;
      if (apu->ch1_env_timer == 0) {
        apu->ch1_env_timer = ep;
        if (get_bit(nr12, 3)) {
          if (apu->ch1_env_vol < 15) apu->ch1_env_vol++;
        } else {
          if (apu->ch1_env_vol > 0) apu->ch1_env_vol--;
        }
      }
    }
    ep = nr22 & 0x07;
    if (ep != 0) {
      if (apu->ch2_env_timer > 0) apu->ch2_env_timer--;
      if (apu->ch2_env_timer == 0) {
        apu->ch2_env_timer = ep;
        if (get_bit(nr22, 3)) {
          if (apu->ch2_env_vol < 15) apu->ch2_env_vol++;
        } else {
          if (apu->ch2_env_vol > 0) apu->ch2_env_vol--;
        }
      }
    }
    ep = nr42 & 0x07;
    if (ep != 0) {
      if (apu->ch4_env_timer > 0) apu->ch4_env_timer--;
      if (apu->ch4_env_timer == 0) {
        apu->ch4_env_timer = ep;
        if (get_bit(nr42, 3)) {
          if (apu->ch4_env_vol < 15) apu->ch4_env_vol++;
        } else {
          if (apu->ch4_env_vol > 0) apu->ch4_env_vol--;
        }
      }
    }
    apu->envelope_clock = 0;
  }
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (apu->ch1_enable) set_bit(&ctrl, 0);
  if (apu->ch2_enable) set_bit(&ctrl, 1);
  if (apu->ch3_enable) set_bit(&ctrl, 2);
  if (apu->ch4_enable) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
  apu->length_clock = 0;
  apu->sweep_clock = 0;
  apu->sample_acc += (uint32_t)cycles * 375u;
  const uint32_t cps = 32768u;
  uint8_t nr51 = mmu_r_mem_raw(mmu, NR51);
  uint8_t nr50 = mmu_r_mem_raw(mmu, NR50);
  uint8_t dac1_on = (nr12 & 0xF8) != 0;
  uint8_t dac2_on = (nr22 & 0xF8) != 0;
  uint8_t dac3_on = (nr30 & 0x80) != 0;
  uint8_t dac4_on = (nr42 & 0xF8) != 0;
  uint8_t any_dac_on = dac1_on | dac2_on | dac3_on | dac4_on;
  while (apu->sample_acc >= cps && apu->sample_count < APU_BUF_SIZE) {
    apu->sample_acc -= cps;
    float L = 0.0f, R = 0.0f;
    if (dac1_on) {
      uint8_t duty1 = (nr11 >> 6) & 3;
      uint8_t dig =
          apu->ch1_enable
              ? (DUTY_TABLE[duty1][apu->ch1_duty_pos] ? apu->ch1_env_vol : 0u)
              : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (nr51 & 0x10) L += s;
      if (nr51 & 0x01) R += s;
    }
    if (dac2_on) {
      uint8_t duty2 = (nr21 >> 6) & 3;
      uint8_t dig =
          apu->ch2_enable
              ? (DUTY_TABLE[duty2][apu->ch2_duty_pos] ? apu->ch2_env_vol : 0u)
              : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (nr51 & 0x20) L += s;
      if (nr51 & 0x02) R += s;
    }
    if (dac3_on) {
      uint8_t vol_code = (mmu_r_mem_raw(mmu, NR32) >> 5) & 3;
      uint8_t shift = vol_code == 0 ? 4 : vol_code - 1;
      uint8_t dig = 0;
      if (apu->ch3_enable) {
        uint8_t wave_byte =
            mmu_r_mem_raw(mmu, (uint16_t)(0xFF30u + apu->ch3_pos / 2));
        uint8_t nibble =
            (apu->ch3_pos & 1) ? (wave_byte & 0x0Fu) : (wave_byte >> 4);
        dig = nibble >> shift;
      }
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (nr51 & 0x40) L += s;
      if (nr51 & 0x04) R += s;
    }
    if (dac4_on) {
      uint8_t dig = apu->ch4_enable
                        ? ((~apu->ch4_lfsr & 1u) ? apu->ch4_env_vol : 0u)
                        : 0u;
      float s = 1.0f - 2.0f * (float)dig / 15.0f;
      if (nr51 & 0x80) L += s;
      if (nr51 & 0x08) R += s;
    }
    float lvol = (float)(((nr50 >> 4) & 7) + 1) / 8.0f;
    float rvol = (float)((nr50 & 7) + 1) / 8.0f;
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

void reset_apu(struct Apu* apu, struct Mmu* mmu) {
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = apu->ch2_len_enable = apu->ch3_len_enable =
      apu->ch4_len_enable = 0;
  apu->sweep_freq = 0;
  apu->sweep_timer = 0;
  apu->sweep_enable = 0;
  apu->sweep_neg_used = 0;
  apu->ch1_freq_timer = 0;
  apu->ch2_freq_timer = 0;
  apu->ch3_freq_timer = 0;
  apu->ch4_freq_timer = 0;
  apu->ch1_duty_pos = 0;
  apu->ch2_duty_pos = 0;
  apu->ch3_pos = 0;
  apu->ch4_lfsr = 0x7FFF;
  apu->ch1_env_vol = 0;
  apu->ch2_env_vol = 0;
  apu->ch4_env_vol = 0;
  apu->ch1_env_timer = 0;
  apu->ch2_env_timer = 0;
  apu->ch4_env_timer = 0;
  mmu_w_mem_raw(mmu, NR10, 0x00);
  mmu_w_mem_raw(mmu, NR11, 0x3F);
  mmu_w_mem_raw(mmu, NR12, 0x00);
  mmu_w_mem_raw(mmu, NR13, 0x00);
  mmu_w_mem_raw(mmu, NR14, 0x00);
  mmu_w_mem_raw(mmu, NR21, 0x3F);
  mmu_w_mem_raw(mmu, NR22, 0x00);
  mmu_w_mem_raw(mmu, NR23, 0x00);
  mmu_w_mem_raw(mmu, NR24, 0x00);
  mmu_w_mem_raw(mmu, NR30, 0x00);
  mmu_w_mem_raw(mmu, NR31, 0xFF);
  mmu_w_mem_raw(mmu, NR32, 0x00);
  mmu_w_mem_raw(mmu, NR33, 0x00);
  mmu_w_mem_raw(mmu, NR34, 0x00);
  // mmu_w_mem(mmu, NR41, 0x00);
  mmu_w_mem_raw(mmu, NR42, 0x00);
  mmu_w_mem_raw(mmu, NR43, 0x00);
  mmu_w_mem_raw(mmu, NR44, 0x00);
  mmu_w_mem_raw(mmu, NR50, 0x00);
  mmu_w_mem_raw(mmu, NR51, 0x00);
  mmu_w_mem_raw(mmu, NR52, 0x00);
}
