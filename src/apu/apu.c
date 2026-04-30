#include "gbemu/apu.h"

#include <stdint.h>
#include <stdlib.h>

#include "apu_private.h"
#include "gbemu/util.h"

struct Apu* apu_init(void) {
  struct Apu* apu = calloc(1, sizeof(struct Apu));
  apu->ch4_lfsr = APU_LFSR_INIT;
  return apu;
}

void apu_free(struct Apu* apu) { free(apu); }

uint8_t apu_read(const struct Apu* apu, uint16_t addr) {
  switch (addr) {
    case 0xFF10:
      return (uint8_t)((unsigned)apu->nr10 | NR10_UNUSED);
    case 0xFF11:
      return (uint8_t)((unsigned)apu->nr11 | NR11_UNUSED);
    case 0xFF12:
      return apu->nr12;
    case 0xFF13:
      return NR13_UNUSED;
    case 0xFF14:
      return (uint8_t)((unsigned)apu->nr14 | NR14_UNUSED);
    case 0xFF15:
      return 0xFF;
    case 0xFF16:
      return (uint8_t)((unsigned)apu->nr21 | NR21_UNUSED);
    case 0xFF17:
      return apu->nr22;
    case 0xFF18:
      return NR23_UNUSED;
    case 0xFF19:
      return (uint8_t)((unsigned)apu->nr24 | NR24_UNUSED);
    case 0xFF1A:
      return (uint8_t)((unsigned)apu->nr30 | NR30_UNUSED);
    case 0xFF1B:
      return NR31_UNUSED;
    case 0xFF1C:
      return (uint8_t)((unsigned)apu->nr32 | NR32_UNUSED);
    case 0xFF1D:
      return NR33_UNUSED;
    case 0xFF1E:
      return (uint8_t)((unsigned)apu->nr34 | NR34_UNUSED);
    case 0xFF1F:
      return 0xFF;
    case 0xFF20:
      return NR41_UNUSED;
    case 0xFF21:
      return apu->nr42;
    case 0xFF22:
      return apu->nr43;
    case 0xFF23:
      return (uint8_t)((unsigned)apu->nr44 | NR44_UNUSED);
    case 0xFF24:
      return apu->nr50;
    case 0xFF25:
      return apu->nr51;
    case 0xFF26:
      return (uint8_t)(((int)apu->powered ? (1U << NR52_POWER_BIT) : 0U) |
                       ((int)apu->ch4_active ? 0x08U : 0U) |
                       ((int)apu->ch3_active ? 0x04U : 0U) |
                       ((int)apu->ch2_active ? 0x02U : 0U) |
                       ((int)apu->ch1_active ? 0x01U : 0U) | NR52_UNUSED_BITS);
    default:
      if (addr >= 0xFF30 && addr <= 0xFF3F) {
        return apu->wave_ram[addr - 0xFF30];
      }
      return 0xFF;
  }
}

static void apu_clock_length(struct Apu* apu) {
  if (apu->ch1_len_enable) {
    if (clock_length_u8(&apu->ch1_len, APU_CH1_LEN_MAX)) {
      apu->ch1_active = false;
    }
  }
  if (apu->ch2_len_enable) {
    if (clock_length_u8(&apu->ch2_len, APU_CH2_LEN_MAX)) {
      apu->ch2_active = false;
    }
  }
  if (apu->ch3_len_enable) {
    if (clock_length_u16(&apu->ch3_len, APU_CH3_LEN_MAX)) {
      apu->ch3_active = false;
    }
  }
  if (apu->ch4_len_enable) {
    if (clock_length_u8(&apu->ch4_len, APU_CH4_LEN_MAX)) {
      apu->ch4_active = false;
    }
  }
}

static void clock_envelope(uint8_t env_period, uint8_t* timer, uint8_t* vol,
                           uint8_t add) {
  if (env_period == 0) {
    return;
  }
  if (*timer > 0) {
    (*timer)--;
  }
  if (*timer == 0) {
    *timer = env_period;
    if (add) {
      if (*vol < 15) {
        (*vol)++;
      }
    } else {
      if (*vol > 0) {
        (*vol)--;
      }
    }
  }
}

static void apu_clock_envelope(struct Apu* apu) {
  clock_envelope((uint8_t)((unsigned)apu->nr12 & 0x07U), &apu->ch1_env_timer,
                 &apu->ch1_env_vol, get_bit(apu->nr12, 3));
  clock_envelope((uint8_t)((unsigned)apu->nr22 & 0x07U), &apu->ch2_env_timer,
                 &apu->ch2_env_vol, get_bit(apu->nr22, 3));
  clock_envelope((uint8_t)((unsigned)apu->nr42 & 0x07U), &apu->ch4_env_timer,
                 &apu->ch4_env_vol, get_bit(apu->nr42, 3));
}

void apu_notify_div_tick(struct Apu* apu) {
  if (!apu->powered) {
    return;
  }
  apu->seq_step = (uint8_t)(((unsigned)apu->seq_step + 1U) & 7U);
  if (((unsigned)apu->seq_step & 1U) == 0U) {
    apu_clock_length(apu);
  }
  if (apu->seq_step == 2 || apu->seq_step == 6) {
    apu_clock_sweep(apu);
  }
  if (apu->seq_step == 7) {
    apu_clock_envelope(apu);
  }
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
  apu->ch4_lfsr = APU_LFSR_INIT;
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

static void apu_write_control(struct Apu* apu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF24:
      apu->nr50 = val;
      break;
    case 0xFF25:
      apu->nr51 = val;
      break;
    case 0xFF26:
      if (!((unsigned)val & (1U << NR52_POWER_BIT)) && apu->powered) {
        apu_power_off(apu);
        apu->powered = false;
      }
      break;
    default:
      break;
  }
}

void apu_write(struct Apu* apu, uint16_t addr, uint8_t val) {
  if (addr >= 0xFF30 && addr <= 0xFF3F) {
    apu->wave_ram[addr - 0xFF30] = val;
    return;
  }
  if (!apu->powered) {
    switch (addr) {
      case 0xFF11:
        apu->ch1_len = (uint8_t)((unsigned)val & (APU_CH1_LEN_MAX - 1U));
        return;
      case 0xFF16:
        apu->ch2_len = (uint8_t)((unsigned)val & (APU_CH2_LEN_MAX - 1U));
        return;
      case 0xFF1B:
        apu->ch3_len = val;
        return;
      case 0xFF20:
        apu->ch4_len = (uint8_t)((unsigned)val & (APU_CH4_LEN_MAX - 1U));
        return;
      case 0xFF26:
        if ((unsigned)val & (1U << NR52_POWER_BIT)) {
          apu->powered = true;
          apu->seq_step = 0;
        }
        return;
      default:
        return;
    }
  }
  if (addr >= 0xFF10 && addr <= 0xFF14) {
    apu_write_ch1(apu, addr, val);
  } else if (addr >= 0xFF16 && addr <= 0xFF19) {
    apu_write_ch2(apu, addr, val);
  } else if (addr >= 0xFF1A && addr <= 0xFF1E) {
    apu_write_ch3(apu, addr, val);
  } else if (addr >= 0xFF20 && addr <= 0xFF23) {
    apu_write_ch4(apu, addr, val);
  } else if (addr >= 0xFF24 && addr <= 0xFF26) {
    apu_write_control(apu, addr, val);
  }
}

void apu_tick(struct Apu* apu, uint8_t cycles) {
  if (!apu->powered) {
    apu->sample_acc += (uint32_t)cycles * 375U;
    const uint32_t cps = 32768U;
    while (apu->sample_acc >= cps && apu->sample_count < APU_BUF_SIZE) {
      apu->sample_acc -= cps;
      apu->sample_buf[apu->sample_count][0] = 0.0F;
      apu->sample_buf[apu->sample_count][1] = 0.0F;
      apu->sample_count++;
    }
    return;
  }
  if (!((unsigned)apu->nr12 & 0xF8U)) {
    apu->ch1_active = false;
  }
  if (!((unsigned)apu->nr22 & 0xF8U)) {
    apu->ch2_active = false;
  }
  if (!((unsigned)apu->nr30 & (1U << NR52_POWER_BIT))) {
    apu->ch3_active = false;
  }
  if (!((unsigned)apu->nr42 & 0xF8U)) {
    apu->ch4_active = false;
  }
  if (apu->sweep_neg_used && !get_bit(apu->nr10, 3)) {
    apu->ch1_active = false;
    apu->sweep_neg_used = false;
  }
  if (apu->ch1_freq_timer > 0) {
    tick_ch1_freq(apu, cycles);
  }
  if (apu->ch2_freq_timer > 0) {
    tick_ch2_freq(apu, cycles);
  }
  if (apu->ch3_freq_timer > 0) {
    tick_ch3_freq(apu, cycles);
  }
  if (apu->ch4_freq_timer > 0) {
    tick_ch4_freq(apu, cycles);
  }
  apu_mix_samples(apu, cycles);
}

void apu_discard_samples(struct Apu* apu) { apu->sample_count = 0; }

uint32_t apu_get_sample_count(const struct Apu* apu) {
  return apu->sample_count;
}

const float (*apu_get_sample_buf(const struct Apu* apu))[2] {
  return apu->sample_buf;
}
