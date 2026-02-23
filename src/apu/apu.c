#include "gbemu/apu.h"

#include "gbemu/mmu.h"
#include "gbemu/util.h"

struct Apu* init_apu() {
  struct Apu* apu = malloc(sizeof(struct Apu));
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = apu->ch1_trigger = 0;
  apu->ch1_len = apu->ch1_len_apu_div = apu->ch1_len_init = 0x00;
  apu->div_apu = 0x00;
  return apu;
}

void upd_apu(struct Apu* apu, struct Mmu* mmu) {
  if (!get_bit(mmu_r_mem(mmu, NR52), 7)) {
    reset_apu(apu, mmu);
    return;
  }
  uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
  uint8_t nr14 = mmu_r_mem_raw(mmu, NR14);
  uint8_t curr_ch1_len = nr11 & 0b111111;
  if (get_bit(nr14, 7)) {
    apu->ch1_len_init = curr_ch1_len;
    if (apu->ch1_len == 0x40) { apu->ch1_len = 0; }
    apu->ch1_len_apu_div = apu->div_apu;
    apu->ch1_enable = 1;
    if (get_bit(nr14, 6)) apu->ch1_len_enable = 1;
  }
  if (apu->ch1_len_init != curr_ch1_len) {
    apu->ch1_len = curr_ch1_len;
    apu->ch1_len_init = curr_ch1_len;
    apu->ch1_len_apu_div = apu->div_apu;
  }
  if (apu->ch1_enable && apu->ch1_len_enable &&
      (uint8_t)(apu->div_apu - apu->ch1_len_apu_div) >= 2) {
    apu->ch1_len++;
    apu->ch1_len_apu_div = apu->div_apu;
    if (apu->ch1_len == 0x40) {
      apu->ch1_enable = 0;
      apu->ch1_len = 0;
      apu->ch1_len_init = curr_ch1_len;
    }
  }
  clear_bit(&nr14, 7);
  mmu_w_mem_raw(mmu, NR14, nr14);
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (apu->ch1_enable) set_bit(&ctrl, 0);
  if (apu->ch2_enable) set_bit(&ctrl, 1);
  if (apu->ch3_enable) set_bit(&ctrl, 2);
  if (apu->ch4_enable) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
}

void reset_apu(struct Apu* apu, struct Mmu* mmu) {
  apu->ch1_enable = apu->ch2_enable = apu->ch3_enable = apu->ch4_enable = 0;
  apu->ch1_len_enable = 0;
  apu->ch1_len = apu->ch1_len_init = 0;
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
  mmu_w_mem(mmu, NR41, 0x00);
  mmu_w_mem(mmu, NR42, 0x00);
  mmu_w_mem(mmu, NR43, 0x00);
  mmu_w_mem(mmu, NR44, 0x00);
  mmu_w_mem(mmu, NR50, 0x00);
  mmu_w_mem(mmu, NR51, 0x00);
  mmu_w_mem(mmu, NR52, 0x00);
}
