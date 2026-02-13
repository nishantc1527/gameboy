#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "internal.h"

int c_adc(uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (gt_flg(FLG_C)) {
    st_h_add(A, 1);
    st_c_add(A, 1);
    cy = gt_flg(FLG_C);
    hy = gt_flg(FLG_H);
    A++;
  }
  c_add(reg);
  if (cy) st_flg(FLG_C);
  if (hy) st_flg(FLG_H);
  return 4;
}

int c_add(uint8_t reg) {
  st_h_add(A, reg);
  st_c_add(A, reg);
  A += reg;
  st_z(A);
  cl_flg(FLG_N);
  return 4;
}

int c_and(uint8_t reg) {
  A &= reg;
  st_z(A);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

int c_bit(uint8_t reg, int bit) {
  if (get_bit(reg, bit))
    cl_flg(FLG_Z);
  else
    st_flg(FLG_Z);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  return 8;
}

int c_call(int flg) {
  if (flg) {
    push(PC + 3);
    PC = rd16();
    kp();
    return 24;
  }
  rd16();
  return 12;
}

int c_cp(uint8_t reg) {
  st_z(A - reg);
  st_flg(FLG_N);
  st_h_sub(A, reg);
  st_c_sub(A, reg);
  return 4;
}

int c_cpl(uint8_t* reg) {
  *reg = ~*reg;
  st_flg(FLG_N);
  st_flg(FLG_H);
  return 4;
}

int c_dec(uint8_t* reg) {
  st_h_sub(*reg, 1);
  *reg = *reg - 1;
  st_z(*reg);
  st_flg(FLG_N);
  return 4;
}

int c_dec_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_sub(reg, 1);
  reg = reg - 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  st_flg(FLG_N);
  return 12;
}

int c_inc(uint8_t* reg) {
  st_h_add(*reg, 1);
  *reg = *reg + 1;
  st_z(*reg);
  cl_flg(FLG_N);
  return 4;
}

int c_inc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_add(reg, 1);
  reg = reg + 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  return 12;
}

int c_jp8(int flg) {
  if (flg) {
    PC += (signed char)rd8();
    return 12;
  }
  rd8();
  return 8;
}

int c_jp16(int flg) {
  if (flg) {
    PC = rd16();
    kp();
    return 16;
  } else
    rd16();
  return 12;
}

int c_or(uint8_t reg) {
  A |= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

int c_res(uint8_t* reg, int bit) {
  *reg = *reg & ~(1 << bit);
  return 8;
}

int c_res_mem(uint16_t loc, int bit) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = reg & ~(1 << bit);
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

int c_ret(int flg) {
  if (flg) {
    PC = pop();
    kp();
    return 20;
  }
  return 8;
}

int c_rr(uint8_t* reg) {
  int carry = gt_flg(FLG_C);
  st_c_rr(*reg);
  *reg = (*reg >> 1) | (carry << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_rr_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  int carry = gt_flg(FLG_C);
  st_c_rr(reg);
  reg = (reg >> 1) | (carry << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_rrc(uint8_t* reg) {
  int carry = *reg & 1;
  st_c_rr(*reg);
  *reg = (*reg >> 1) | (carry << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_rrc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  int carry = reg & 1;
  st_c_rr(reg);
  reg = (reg >> 1) | (carry << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_rl(uint8_t* reg) {
  int carry = gt_flg(FLG_C);
  st_c_rl(*reg);
  *reg = (*reg << 1) | carry;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_rl_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  int carry = gt_flg(FLG_C);
  st_c_rl(reg);
  reg = (reg << 1) | carry;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_rlc(uint8_t* reg) {
  int carry = (*reg >> 7) & 1;
  st_c_rl(*reg);
  *reg = (*reg << 1) | carry;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_rlc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  int carry = (reg >> 7) & 1;
  st_c_rl(reg);
  reg = (reg << 1) | carry;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_rst(uint8_t loc) {
  push(PC + 1);
  PC = loc;
  kp();
  return 16;
}

int c_sbc(uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (gt_flg(FLG_C)) {
    st_h_sub(A, 1);
    st_c_sub(A, 1);
    cy = gt_flg(FLG_C);
    hy = gt_flg(FLG_H);
    A--;
  }
  c_sub(reg);
  if (cy) st_flg(FLG_C);
  if (hy) st_flg(FLG_H);
  return 4;
}

int c_srl(uint8_t* reg) {
  if (*reg & 1)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
  *reg = *reg >> 1;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_srl_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  if (reg & 1)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
  reg = reg >> 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_set(uint8_t* reg, int bit) {
  set_bit(reg, bit);
  return 8;
}

int c_set_mem(uint16_t loc, int bit) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  set_bit(&reg, bit);
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

int c_sla(uint8_t* reg) {
  st_c_rl(*reg);
  *reg = *reg << 1;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_sla_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rl(reg);
  reg = reg << 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_sra(uint8_t* reg) {
  st_c_rr(*reg);
  int bt = (*reg >> 7) & 1;
  *reg = (*reg >> 1) | (bt << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_sra_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rr(reg);
  int bt = (reg >> 7) & 1;
  reg = (reg >> 1) | (bt << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

int c_sub(uint8_t reg) {
  st_h_sub(A, reg);
  st_c_sub(A, reg);
  A -= reg;
  st_z(A);
  st_flg(FLG_N);
  return 4;
}

int c_swp(uint8_t* reg) {
  *reg = (*reg >> 4) | (*reg << 4);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 8;
}

int c_swp_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = (reg >> 4) | (reg << 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 16;
}

int c_xor(int reg) {
  A ^= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}
