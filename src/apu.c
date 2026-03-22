#include "gbemu/apu.h"

#include <stdlib.h>

#include "gbemu/util.h"

#define HPF_CHARGE 0.99634f

static const uint8_t DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

struct Apu* apu_init(void) {
  struct Apu* apu = calloc(1, sizeof(struct Apu));
  apu->ch4_lfsr = 0x7FFF;
  return apu;
}

void apu_free(struct Apu* apu) { free(apu); }

uint8_t apu_read(const struct Apu* apu, uint16_t addr) {
  switch (addr) {
    case 0xFF10:
      return apu->nr10 | 0x80;
    case 0xFF11:
      return apu->nr11 | 0x3F;
    case 0xFF12:
      return apu->nr12;
    case 0xFF13:
      return 0xFF;
    case 0xFF14:
      return apu->nr14 | 0xBF;
    case 0xFF15:
      return 0xFF;
    case 0xFF16:
      return apu->nr21 | 0x3F;
    case 0xFF17:
      return apu->nr22;
    case 0xFF18:
      return 0xFF;
    case 0xFF19:
      return apu->nr24 | 0xBF;
    case 0xFF1A:
      return apu->nr30 | 0x7F;
    case 0xFF1B:
      return 0xFF;
    case 0xFF1C:
      return apu->nr32 | 0x9F;
    case 0xFF1D:
      return 0xFF;
    case 0xFF1E:
      return apu->nr34 | 0xBF;
    case 0xFF1F:
      return 0xFF;
    case 0xFF20:
      return 0xFF;
    case 0xFF21:
      return apu->nr42;
    case 0xFF22:
      return apu->nr43;
    case 0xFF23:
      return apu->nr44 | 0xBF;
    case 0xFF24:
      return apu->nr50;
    case 0xFF25:
      return apu->nr51;
    case 0xFF26:
      return (apu->powered ? 0x80u : 0u) | (apu->ch4_active ? 0x08u : 0u) |
             (apu->ch3_active ? 0x04u : 0u) | (apu->ch2_active ? 0x02u : 0u) |
             (apu->ch1_active ? 0x01u : 0u) | 0x70u;
    default:
      if (addr >= 0xFF30 && addr <= 0xFF3F) return apu->wave_ram[addr - 0xFF30];
      return 0xFF;
  }
}

static uint8_t clock_length_u8(uint8_t* len, uint8_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}

static uint8_t clock_length_u16(uint16_t* len, uint16_t max) {
  (*len)++;
  return (*len >= max) ? 1 : 0;
}

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

static void apu_clock_length(struct Apu* apu) {
  if (apu->ch1_len_enable)
    if (clock_length_u8(&apu->ch1_len, 0x40)) apu->ch1_active = false;
  if (apu->ch2_len_enable)
    if (clock_length_u8(&apu->ch2_len, 0x40)) apu->ch2_active = false;
  if (apu->ch3_len_enable)
    if (clock_length_u16(&apu->ch3_len, 0x100)) apu->ch3_active = false;
  if (apu->ch4_len_enable)
    if (clock_length_u8(&apu->ch4_len, 0x40)) apu->ch4_active = false;
}

static void apu_clock_sweep(struct Apu* apu) {
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

static void apu_clock_envelope(struct Apu* apu) {
  uint8_t ep;
  ep = apu->nr12 & 0x07;
  if (ep != 0) {
    if (apu->ch1_env_timer > 0) apu->ch1_env_timer--;
    if (apu->ch1_env_timer == 0) {
      apu->ch1_env_timer = ep;
      if (get_bit(apu->nr12, 3)) {
        if (apu->ch1_env_vol < 15) apu->ch1_env_vol++;
      } else {
        if (apu->ch1_env_vol > 0) apu->ch1_env_vol--;
      }
    }
  }
  ep = apu->nr22 & 0x07;
  if (ep != 0) {
    if (apu->ch2_env_timer > 0) apu->ch2_env_timer--;
    if (apu->ch2_env_timer == 0) {
      apu->ch2_env_timer = ep;
      if (get_bit(apu->nr22, 3)) {
        if (apu->ch2_env_vol < 15) apu->ch2_env_vol++;
      } else {
        if (apu->ch2_env_vol > 0) apu->ch2_env_vol--;
      }
    }
  }
  ep = apu->nr42 & 0x07;
  if (ep != 0) {
    if (apu->ch4_env_timer > 0) apu->ch4_env_timer--;
    if (apu->ch4_env_timer == 0) {
      apu->ch4_env_timer = ep;
      if (get_bit(apu->nr42, 3)) {
        if (apu->ch4_env_vol < 15) apu->ch4_env_vol++;
      } else {
        if (apu->ch4_env_vol > 0) apu->ch4_env_vol--;
      }
    }
  }
}

void apu_notify_div_tick(struct Apu* apu) {
  if (!apu->powered) return;
  apu->seq_step = (apu->seq_step + 1) & 7;
  if ((apu->seq_step & 1) == 0) apu_clock_length(apu);
  if (apu->seq_step == 2 || apu->seq_step == 6) apu_clock_sweep(apu);
  if (apu->seq_step == 7) apu_clock_envelope(apu);
}

static void apu_power_off(struct Apu* apu) {
  apu->ch1_active = apu->ch2_active = apu->ch3_active = apu->ch4_active = false;
  apu->ch1_len_enable = apu->ch2_len_enable = apu->ch3_len_enable =
      apu->ch4_len_enable = false;
  apu->sweep_freq = 0;
  apu->sweep_timer = 0;
  apu->sweep_enable = false;
  apu->sweep_neg_used = false;
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
  apu->nr10 = 0;
  apu->nr11 = 0;
  apu->nr12 = 0;
  apu->nr13 = 0;
  apu->nr14 = 0;
  apu->nr21 = 0;
  apu->nr22 = 0;
  apu->nr23 = 0;
  apu->nr24 = 0;
  apu->nr30 = 0;
  apu->nr32 = 0;
  apu->nr33 = 0;
  apu->nr34 = 0;
  apu->nr42 = 0;
  apu->nr43 = 0;
  apu->nr44 = 0;
  apu->nr50 = 0;
  apu->nr51 = 0;
}

void apu_write(struct Apu* apu, uint16_t addr, uint8_t val) {
  if (addr >= 0xFF30 && addr <= 0xFF3F) {
    apu->wave_ram[addr - 0xFF30] = val;
    return;
  }
  if (!apu->powered) {
    switch (addr) {
      case 0xFF11:
        apu->ch1_len = val & 0x3F;
        return;
      case 0xFF16:
        apu->ch2_len = val & 0x3F;
        return;
      case 0xFF1B:
        apu->ch3_len = val;
        return;
      case 0xFF20:
        apu->ch4_len = val & 0x3F;
        return;
      case 0xFF26:
        if (val & 0x80) {
          apu->powered = true;
          apu->seq_step = 0;
        }
        return;
      default:
        return;
    }
  }
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
      apu->ch1_len = val & 0x3F;
      break;
    case 0xFF12:
      apu->nr12 = val;
      if (!(val & 0xF8)) apu->ch1_active = false;
      break;
    case 0xFF13:
      apu->nr13 = val;
      break;
    case 0xFF14: {
      uint8_t old_len_enable = apu->ch1_len_enable;
      apu->ch1_len_enable = (val >> 6) & 1;
      if (!old_len_enable && apu->ch1_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch1_len, 0x40)) apu->ch1_active = false;
      }
      apu->nr14 = val & 0x7F;
      if (val & 0x80) {
        if (apu->ch1_len >= 0x40) {
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
    case 0xFF16:
      apu->nr21 = val;
      apu->ch2_len = val & 0x3F;
      break;
    case 0xFF17:
      apu->nr22 = val;
      if (!(val & 0xF8)) apu->ch2_active = false;
      break;
    case 0xFF18:
      apu->nr23 = val;
      break;
    case 0xFF19: {
      uint8_t old_len_enable = apu->ch2_len_enable;
      apu->ch2_len_enable = (val >> 6) & 1;
      if (!old_len_enable && apu->ch2_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch2_len, 0x40)) apu->ch2_active = false;
      }
      apu->nr24 = val & 0x7F;
      if (val & 0x80) {
        if (apu->ch2_len >= 0x40) {
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
    case 0xFF1A:
      apu->nr30 = val;
      if (!(val & 0x80)) apu->ch3_active = false;
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
      uint8_t old_len_enable = apu->ch3_len_enable;
      apu->ch3_len_enable = (val >> 6) & 1;
      if (!old_len_enable && apu->ch3_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u16(&apu->ch3_len, 0x100)) apu->ch3_active = false;
      }
      apu->nr34 = val & 0x7F;
      if (val & 0x80) {
        if (apu->ch3_len >= 0x100) {
          apu->ch3_len =
              (apu->ch3_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr30 & 0x80) apu->ch3_active = true;
        uint16_t freq3 = ((uint16_t)(apu->nr34 & 0x07) << 8) | apu->nr33;
        apu->ch3_freq_timer = (uint16_t)((2048u - freq3) * 2u);
        apu->ch3_pos = 0;
      }
      break;
    }
    case 0xFF20:
      apu->nr41 = val;
      apu->ch4_len = val & 0x3F;
      break;
    case 0xFF21:
      apu->nr42 = val;
      if (!(val & 0xF8)) apu->ch4_active = false;
      break;
    case 0xFF22:
      apu->nr43 = val;
      break;
    case 0xFF23: {
      uint8_t old_len_enable = apu->ch4_len_enable;
      apu->ch4_len_enable = (val >> 6) & 1;
      if (!old_len_enable && apu->ch4_len_enable && (apu->seq_step & 1) == 0) {
        if (clock_length_u8(&apu->ch4_len, 0x40)) apu->ch4_active = false;
      }
      apu->nr44 = val & 0x7F;
      if (val & 0x80) {
        if (apu->ch4_len >= 0x40) {
          apu->ch4_len =
              (apu->ch4_len_enable && (apu->seq_step & 1) == 0) ? 1 : 0;
        }
        if (apu->nr42 & 0xF8) apu->ch4_active = true;
        apu->ch4_lfsr = 0x7FFF;
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
    case 0xFF24:
      apu->nr50 = val;
      break;
    case 0xFF25:
      apu->nr51 = val;
      break;
    case 0xFF26:
      if (!(val & 0x80) && apu->powered) {
        apu_power_off(apu);
        apu->powered = false;
      }
      break;
    default:
      break;
  }
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

void apu_tick(struct Apu* apu, uint8_t cycles) {
  if (!apu->powered) {
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
  if (!(apu->nr12 & 0xF8)) apu->ch1_active = false;
  if (!(apu->nr22 & 0xF8)) apu->ch2_active = false;
  if (!(apu->nr30 & 0x80)) apu->ch3_active = false;
  if (!(apu->nr42 & 0xF8)) apu->ch4_active = false;
  if (apu->sweep_neg_used && !get_bit(apu->nr10, 3)) {
    apu->ch1_active = false;
    apu->sweep_neg_used = false;
  }
  if (apu->ch1_freq_timer > 0) {
    uint16_t freq1 = ((uint16_t)(apu->nr14 & 0x07) << 8) | apu->nr13;
    uint16_t period1 = (uint16_t)((2048u - freq1) * 4u);
    if (period1 == 0) period1 = 1;
    TICK_FREQ16(apu->ch1_freq_timer, cycles, period1,
                apu->ch1_duty_pos = (apu->ch1_duty_pos + 1) & 7);
  }
  if (apu->ch2_freq_timer > 0) {
    uint16_t freq2 = ((uint16_t)(apu->nr24 & 0x07) << 8) | apu->nr23;
    uint16_t period2 = (uint16_t)((2048u - freq2) * 4u);
    if (period2 == 0) period2 = 1;
    TICK_FREQ16(apu->ch2_freq_timer, cycles, period2,
                apu->ch2_duty_pos = (apu->ch2_duty_pos + 1) & 7);
  }
  if (apu->ch3_freq_timer > 0) {
    uint16_t freq3 = ((uint16_t)(apu->nr34 & 0x07) << 8) | apu->nr33;
    uint16_t period3 = (uint16_t)((2048u - freq3) * 2u);
    if (period3 == 0) period3 = 1;
    TICK_FREQ16(apu->ch3_freq_timer, cycles, period3,
                apu->ch3_pos = (apu->ch3_pos + 1) & 31);
  }
  if (apu->ch4_freq_timer > 0) {
    uint8_t r4 = apu->nr43 & 0x07;
    uint8_t s4 = (apu->nr43 >> 4) & 0x0F;
    uint32_t period4 = (uint32_t)(r4 == 0 ? 8u : (uint32_t)r4 * 16u) << s4;
    if (period4 == 0) period4 = 1;
    TICK_FREQ32(apu->ch4_freq_timer, cycles, period4, {
      uint8_t xb =
          (uint8_t)((apu->ch4_lfsr & 1u) ^ ((apu->ch4_lfsr >> 1) & 1u));
      apu->ch4_lfsr >>= 1;
      apu->ch4_lfsr |= (uint16_t)(xb << 14);
      if (apu->nr43 & 0x08) {
        apu->ch4_lfsr &= (uint16_t)~(1u << 6);
        apu->ch4_lfsr |= (uint16_t)(xb << 6);
      }
    });
  }

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
