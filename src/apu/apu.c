#include "gbemu/apu.h"

#include "gbemu/mmu.h"
#include "gbemu/util.h"

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

void upd_apu(struct Apu* apu, struct Mmu* mmu) {
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

  if (!curr_apu_on) {
    uint8_t nr41 = mmu_r_mem_raw(mmu, NR41);
    apu->ch4_len = nr41 & 0x3F;
    apu->length_clock = 0;
    apu->sweep_clock = 0;
    return;
  }

  uint8_t nr10 = mmu_r_mem_raw(mmu, NR10);
  uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
  uint8_t nr12 = mmu_r_mem(mmu, NR12);
  uint8_t nr13 = mmu_r_mem_raw(mmu, NR13);
  uint8_t nr14 = mmu_r_mem_raw(mmu, NR14);
  uint8_t curr_ch1_len_enable = get_bit(nr14, 6);
  uint8_t ch1_triggered = get_bit(nr14, 7);
  if ((nr11 & 0x3F) != 0x3F) apu->ch1_len = nr11 & 0x3F;
  if (ch1_triggered && (nr12 & 0xF8)) {
    apu->ch1_enable = 1;
    if (apu->ch1_len >= 0x40) apu->ch1_len = 0;
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
  uint8_t nr21 = mmu_r_mem_raw(mmu, NR21);
  uint8_t nr22 = mmu_r_mem(mmu, NR22);
  uint8_t nr24 = mmu_r_mem_raw(mmu, NR24);
  uint8_t curr_ch2_len_enable = get_bit(nr24, 6);
  uint8_t ch2_triggered = get_bit(nr24, 7);
  if ((nr21 & 0x3F) != 0x3F) apu->ch2_len = nr21 & 0x3F;
  if (ch2_triggered && (nr22 & 0xF8)) {
    apu->ch2_enable = 1;
    if (apu->ch2_len >= 0x40) apu->ch2_len = 0;
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
  uint8_t nr30 = mmu_r_mem_raw(mmu, NR30);
  uint8_t nr31 = mmu_r_mem_raw(mmu, NR31);
  uint8_t nr34 = mmu_r_mem_raw(mmu, NR34);
  uint8_t curr_ch3_len_enable = get_bit(nr34, 6);
  uint8_t ch3_triggered = get_bit(nr34, 7);
  if (nr31 != 0xFF) apu->ch3_len = nr31;
  if (ch3_triggered && (nr30 & 0x80)) {
    apu->ch3_enable = 1;
    if (apu->ch3_len >= 0x100) apu->ch3_len = 0;
  }
  apu->ch3_len_enable = curr_ch3_len_enable;
  if (apu->length_clock && apu->ch3_len_enable) {
    if (clock_length_u16(&apu->ch3_len, 0x100)) apu->ch3_enable = 0;
  }
  clear_bit(&nr34, 7);
  mmu_w_mem_raw(mmu, NR34, nr34);
  mmu_w_mem_raw(mmu, NR31, 0xFF);
  if (!(nr30 & 0x80)) apu->ch3_enable = 0;
  uint8_t nr41 = mmu_r_mem_raw(mmu, NR41);
  uint8_t nr42 = mmu_r_mem(mmu, NR42);
  uint8_t nr44 = mmu_r_mem_raw(mmu, NR44);
  uint8_t curr_ch4_len_enable = get_bit(nr44, 6);
  uint8_t ch4_triggered = get_bit(nr44, 7);
  if ((nr41 & 0x3F) != 0x3F) apu->ch4_len = nr41 & 0x3F;
  if (ch4_triggered && (nr42 & 0xF8)) {
    apu->ch4_enable = 1;
    if (apu->ch4_len >= 0x40) apu->ch4_len = 0;
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
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (apu->ch1_enable) set_bit(&ctrl, 0);
  if (apu->ch2_enable) set_bit(&ctrl, 1);
  if (apu->ch3_enable) set_bit(&ctrl, 2);
  if (apu->ch4_enable) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
  apu->length_clock = 0;
  apu->sweep_clock = 0;
}

void reset_apu(struct Apu* apu, struct Mmu* mmu) {
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = apu->ch2_len_enable = apu->ch3_len_enable =
      apu->ch4_len_enable = 0;
  apu->sweep_freq = 0;
  apu->sweep_timer = 0;
  apu->sweep_enable = 0;
  apu->sweep_neg_used = 0;
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
