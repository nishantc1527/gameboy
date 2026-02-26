#include "gbemu/apu.h"

#include "gbemu/mmu.h"
#include "gbemu/util.h"

struct Apu* init_apu() {
  struct Apu* apu = malloc(sizeof(struct Apu));
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = 0;
  apu->ch1_len = apu->ch1_len_apu_div = apu->ch1_len_init = 0x00;
  apu->ch2_len_enable = 0;
  apu->ch2_len = apu->ch2_len_apu_div = apu->ch2_len_init = 0x00;
  apu->ch3_len_enable = 0;
  apu->ch3_len = apu->ch3_len_apu_div = apu->ch3_len_init = 0x00;
  apu->ch4_len_enable = 0;
  apu->ch4_len = apu->ch4_len_apu_div = apu->ch4_len_init = 0x00;
  apu->div_apu = 0x00;
  return apu;
}

void upd_apu(struct Apu* apu, struct Mmu* mmu) {
  if (!get_bit(mmu_r_mem(mmu, NR52), 7)) reset_apu(apu, mmu);
  uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
  uint8_t nr12 = mmu_r_mem(mmu, NR12);
  uint8_t nr14 = mmu_r_mem_raw(mmu, NR14);
  uint8_t curr_ch1_len_init = nr11 & 0x3F;
  if (get_bit(nr14, 7) && (nr12 & 0xF8)) {
    apu->ch1_enable = 1;
    if (apu->ch1_len >= 0x40 || !apu->ch1_len_enable) {
      apu->ch1_len = 0;
      apu->ch1_len_apu_div = apu->div_apu;
    }
  }
  if (curr_ch1_len_init != 0x3F) {
    apu->ch1_len = curr_ch1_len_init;
    apu->ch1_len_init = curr_ch1_len_init;
    apu->ch1_len_apu_div = apu->div_apu;
  }
  uint8_t curr_ch1_len_enable = get_bit(nr14, 6);
  if (!apu->ch1_len_enable && curr_ch1_len_enable &&
      apu->div_apu == apu->ch1_len_apu_div && apu->ch1_len < 0x40) {
    // apu->div_apu++;
  }
  apu->ch1_len_enable = curr_ch1_len_enable;
  if ((uint8_t)(apu->div_apu - apu->ch1_len_apu_div) >= 2) {
    apu->ch1_len_apu_div = apu->div_apu;
    if (apu->ch1_len_enable) {
      apu->ch1_len++;
      if (apu->ch1_len >= 0x40) apu->ch1_enable = 0;
    }
  }
  clear_bit(&nr14, 7);
  mmu_w_mem_raw(mmu, NR14, nr14);
  nr11 |= 0x3F;
  mmu_w_mem_raw(mmu, NR11, nr11);
  if (nr12 <= 0x07) apu->ch1_enable = 0;
  uint8_t nr21 = mmu_r_mem_raw(mmu, NR21);
  uint8_t nr22 = mmu_r_mem(mmu, NR22);
  uint8_t nr24 = mmu_r_mem_raw(mmu, NR24);
  uint8_t curr_ch2_len_init = nr21 & 0x3F;
  apu->ch2_len_enable = get_bit(nr24, 6);
  if (get_bit(nr24, 7) && (nr22 & 0xF8)) {
    apu->ch2_enable = 1;
    if (apu->ch2_len >= 0x40) {
      apu->ch2_len = 0;
      apu->ch2_len_apu_div = apu->div_apu;
    }
  }
  if (curr_ch2_len_init != 0x3F) {
    apu->ch2_len = curr_ch2_len_init;
    apu->ch2_len_init = curr_ch2_len_init;
    apu->ch2_len_apu_div = apu->div_apu;
  }
  if (!apu->ch2_len_enable) apu->ch2_len_apu_div = apu->div_apu;
  if (apu->ch2_len_enable &&
      (uint8_t)(apu->div_apu - apu->ch2_len_apu_div) >= 2) {
    apu->ch2_len++;
    apu->ch2_len_apu_div = apu->div_apu;
    if (apu->ch2_len >= 0x40) apu->ch2_enable = 0;
  }
  clear_bit(&nr24, 7);
  mmu_w_mem_raw(mmu, NR24, nr24);
  nr21 |= 0x3F;
  mmu_w_mem_raw(mmu, NR21, nr21);
  if (nr22 <= 0x07) apu->ch2_enable = 0;
  uint8_t nr30 = mmu_r_mem_raw(mmu, NR30);
  uint8_t nr31 = mmu_r_mem_raw(mmu, NR31);
  uint8_t nr34 = mmu_r_mem_raw(mmu, NR34);
  uint8_t curr_ch3_len_init = nr31;
  apu->ch3_len_enable = get_bit(nr34, 6);
  if (get_bit(nr34, 7) && (nr30 & 0x80)) {
    apu->ch3_enable = 1;
    if (apu->ch3_len >= 0x100) {
      apu->ch3_len = 0;
      apu->ch3_len_apu_div = apu->div_apu;
    }
  }
  if (curr_ch3_len_init != 0xFF) {
    apu->ch3_len = curr_ch3_len_init;
    apu->ch3_len_init = curr_ch3_len_init;
    apu->ch3_len_apu_div = apu->div_apu;
  }
  if (!apu->ch3_len_enable) apu->ch3_len_apu_div = apu->div_apu;
  if (apu->ch3_len_enable &&
      (uint8_t)(apu->div_apu - apu->ch3_len_apu_div) >= 2) {
    apu->ch3_len++;
    apu->ch3_len_apu_div = apu->div_apu;
    if (apu->ch3_len >= 0x100) apu->ch3_enable = 0;
  }
  clear_bit(&nr34, 7);
  mmu_w_mem_raw(mmu, NR34, nr34);
  mmu_w_mem_raw(mmu, NR31, 0xFF);
  if (!(nr30 & 0x80)) apu->ch3_enable = 0;
  uint8_t nr41 = mmu_r_mem_raw(mmu, NR41);
  uint8_t nr42 = mmu_r_mem(mmu, NR42);
  uint8_t nr44 = mmu_r_mem_raw(mmu, NR44);
  uint8_t curr_ch4_len_init = nr41 & 0x3F;
  apu->ch4_len_enable = get_bit(nr44, 6);
  if (get_bit(nr44, 7) && (nr42 & 0xF8)) {
    apu->ch4_enable = 1;
    if (apu->ch4_len >= 0x40) {
      apu->ch4_len = 0;
      apu->ch4_len_apu_div = apu->div_apu;
    }
  }
  if (curr_ch4_len_init != 0x3F) {
    apu->ch4_len = curr_ch4_len_init;
    apu->ch4_len_init = curr_ch4_len_init;
    apu->ch4_len_apu_div = apu->div_apu;
  }
  if (!apu->ch4_len_enable) apu->ch4_len_apu_div = apu->div_apu;
  if (apu->ch4_len_enable &&
      (uint8_t)(apu->div_apu - apu->ch4_len_apu_div) >= 2) {
    apu->ch4_len++;
    apu->ch4_len_apu_div = apu->div_apu;
    if (apu->ch4_len >= 0x40) apu->ch4_enable = 0;
  }
  clear_bit(&nr44, 7);
  mmu_w_mem_raw(mmu, NR44, nr44);
  nr41 |= 0x3F;
  mmu_w_mem_raw(mmu, NR41, nr41);
  if (nr42 <= 0x07) apu->ch4_enable = 0;
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (apu->ch1_enable) set_bit(&ctrl, 0);
  if (apu->ch2_enable) set_bit(&ctrl, 1);
  if (apu->ch3_enable) set_bit(&ctrl, 2);
  if (apu->ch4_enable) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
}

void reset_apu(struct Apu* apu, struct Mmu* mmu) {
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = apu->ch2_len_enable = apu->ch3_len_enable =
      apu->ch4_len_enable = 0;
  apu->ch1_len = apu->ch1_len_init = apu->ch2_len = apu->ch2_len_init =
      apu->ch3_len = apu->ch3_len_init = 0x00;
  mmu_w_mem(mmu, NR10, 0x00);
  mmu_w_mem(mmu, NR11, 0x00);
  mmu_w_mem(mmu, NR12, 0x00);
  mmu_w_mem(mmu, NR13, 0x00);
  mmu_w_mem(mmu, NR14, 0x00);
  mmu_w_mem(mmu, NR21, 0x00);
  mmu_w_mem(mmu, NR22, 0x00);
  mmu_w_mem(mmu, NR23, 0x00);
  mmu_w_mem(mmu, NR24, 0x00);
  mmu_w_mem(mmu, NR30, 0x00);
  mmu_w_mem(mmu, NR31, 0x00);
  mmu_w_mem(mmu, NR32, 0x00);
  mmu_w_mem(mmu, NR33, 0x00);
  mmu_w_mem(mmu, NR34, 0x00);
  // mmu_w_mem(mmu, NR41, 0x00);
  mmu_w_mem(mmu, NR42, 0x00);
  mmu_w_mem(mmu, NR43, 0x00);
  mmu_w_mem(mmu, NR44, 0x00);
  mmu_w_mem(mmu, NR50, 0x00);
  mmu_w_mem(mmu, NR51, 0x00);
  mmu_w_mem(mmu, NR52, 0x00);
}
