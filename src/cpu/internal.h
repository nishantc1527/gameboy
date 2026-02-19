#pragma once

#include <stdint.h>
#include "gbemu/cpu.h"
#include "gbemu/util.h"
#include "gbemu/mmu.h"

extern uint8_t bHALT, bIME;
extern uint8_t A, B, C, D, E, F, H, L;
extern uint16_t SP;
extern uint32_t tim_thresh;

#define FLG_Z 7
#define FLG_N 6
#define FLG_H 5
#define FLG_C 4

#define INTR_VBLANK 0
#define INTR_LCD 1
#define INTR_TIMER 2
#define INTR_SERIAL 3
#define INTR_JOYPAD 4

extern uint32_t tim_cnt, div_cnt;
extern uint16_t intr_loc[];

static inline uint8_t gt_flg(uint8_t flg) { return gt(F, flg); }
static inline void st_flg(uint8_t flg) { st(&F, flg); }
static inline void cl_flg(uint8_t flg) { cl(&F, flg); }

static inline uint8_t rd8(void) { return mmu_r_mem(mmu, ++PC); }

static inline uint16_t rd16(void) {
  uint16_t addr1 = ++PC;
  uint16_t addr2 = ++PC;
  return (uint16_t)((uint16_t)mmu_r_mem(mmu, addr2) << 8) |
         (uint16_t)mmu_r_mem(mmu, addr1);
}

static inline void push(uint16_t val) {
  uint8_t val1 = (uint8_t)(val >> 8);
  uint8_t val2 = (uint8_t)val;
  mmu_w_mem(mmu, SP - 1, val1);
  mmu_w_mem(mmu, SP - 2, val2);
  SP -= 2;
}

static inline uint16_t pop(void) {
  uint16_t val1 = mmu_r_mem(mmu, SP);
  uint16_t val2 = mmu_r_mem(mmu, SP + 1);
  SP += 2;
  return val1 | (uint16_t)(val2 << 8);
}

static inline uint16_t pk(void) {
  uint16_t val = pop();
  push(val);
  return val;
}

static inline void kp(void) { PC--; }

static inline void st_z(uint8_t var) {
  if (var == 0)
    st_flg(FLG_Z);
  else
    cl_flg(FLG_Z);
}
static inline void st_h_add(uint8_t var1, uint8_t var2) {
  if (((var1 & 0xF) + (var2 & 0xF)) & 0x10)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}
static inline void st_h_add16(uint16_t var1, uint16_t var2) {
  if (((var1 & 0xFFF) + (var2 & 0xFFF)) & 0x1000)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}
static inline void st_h_sub(uint8_t var1, uint8_t var2) {
  if (((var1 & 0x0F) - (var2 & 0x0F)) & 0x10)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}
static inline void st_c_rl(uint8_t var) {
  if (var >> 7)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
static inline void st_c_rr(uint8_t var) {
  if (var & 1)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
static inline void st_c_add(uint8_t var1, uint8_t var2) {
  uint16_t res = (uint16_t)var1 + (uint16_t)var2;
  if (res > 0xFF)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
static inline void st_c_add16(uint16_t var1, uint16_t var2) {
  int res = (int)var1 + (int)var2;
  if (res > 0xFFFF)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
static inline void st_c_sub(uint8_t var1, uint8_t var2) {
  if (var1 < var2)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
static inline void req_intr(uint8_t intr) {
  uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
  st(&val, intr);
  mmu_w_mem_raw(mmu, 0xFF0F, val);
}

static inline int c_add(uint8_t reg) {
  st_h_add(A, reg);
  st_c_add(A, reg);
  A += reg;
  st_z(A);
  cl_flg(FLG_N);
  return 4;
}

static inline int c_adc(uint8_t reg) {
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

static inline int c_and(uint8_t reg) {
  A &= reg;
  st_z(A);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

static inline int c_bit(uint8_t reg, uint8_t bit) {
  if (gt(reg, bit))
    cl_flg(FLG_Z);
  else
    st_flg(FLG_Z);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  return 8;
}

static inline int c_call(int flg) {
  if (flg) {
    push(PC + 3);
    PC = rd16();
    kp();
    return 24;
  }
  rd16();
  return 12;
}

static inline int c_cp(uint8_t reg) {
  st_z(A - reg);
  st_flg(FLG_N);
  st_h_sub(A, reg);
  st_c_sub(A, reg);
  return 4;
}

static inline int c_cpl(uint8_t* reg) {
  *reg = ~*reg;
  st_flg(FLG_N);
  st_flg(FLG_H);
  return 4;
}

static inline int c_dec(uint8_t* reg) {
  st_h_sub(*reg, 1);
  *reg = *reg - 1;
  st_z(*reg);
  st_flg(FLG_N);
  return 4;
}

static inline int c_dec_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_sub(reg, 1);
  reg = reg - 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  st_flg(FLG_N);
  return 12;
}

static inline int c_inc(uint8_t* reg) {
  st_h_add(*reg, 1);
  *reg = *reg + 1;
  st_z(*reg);
  cl_flg(FLG_N);
  return 4;
}

static inline int c_inc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_add(reg, 1);
  reg = reg + 1;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  return 12;
}

static inline int c_jp8(int flg) {
  if (flg) {
    PC = (uint16_t)(PC + (int8_t)rd8());
    return 12;
  }
  rd8();
  return 8;
}

static inline int c_jp16(int flg) {
  if (flg) {
    PC = rd16();
    kp();
    return 16;
  } else
    rd16();
  return 12;
}

static inline int c_or(uint8_t reg) {
  A |= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

static inline int c_res(uint8_t* reg, uint8_t bit) {
  *reg = (uint8_t)(*reg & ~(1 << bit));
  return 8;
}

static inline int c_res_mem(uint16_t loc, int bit) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = (uint8_t)(reg & ~(1 << bit));
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

static inline int c_ret(int flg) {
  if (flg) {
    PC = pop();
    kp();
    return 20;
  }
  return 8;
}

static inline int c_rr(uint8_t* reg) {
  uint8_t carry = gt_flg(FLG_C);
  st_c_rr(*reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_rr_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = gt_flg(FLG_C);
  st_c_rr(reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

static inline int c_rrc(uint8_t* reg) {
  uint8_t carry = *reg & 1;
  st_c_rr(*reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_rrc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = reg & 1;
  st_c_rr(reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

static inline int c_rl(uint8_t* reg) {
  uint8_t carry = gt_flg(FLG_C);
  st_c_rl(*reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_rl_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = gt_flg(FLG_C);
  st_c_rl(reg);
  reg = (uint8_t)(reg << 1) | carry;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

static inline int c_rlc(uint8_t* reg) {
  uint8_t carry = (*reg >> 7) & 1;
  st_c_rl(*reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_rlc_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = (reg >> 7) & 1;
  st_c_rl(reg);
  reg = (uint8_t)(reg << 1) | carry;
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

static inline int c_rst(uint8_t loc) {
  push(PC + 1);
  PC = loc;
  kp();
  return 16;
}

static inline int c_sub(uint8_t reg) {
  st_h_sub(A, reg);
  st_c_sub(A, reg);
  A -= reg;
  st_z(A);
  st_flg(FLG_N);
  return 4;
}

static inline int c_sbc(uint8_t reg) {
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

static inline int c_srl(uint8_t* reg) {
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

static inline int c_srl_mem(uint16_t loc) {
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

static inline int c_set(uint8_t* reg, uint8_t bit) {
  st(reg, bit);
  return 8;
}

static inline int c_set_mem(uint16_t loc, uint8_t bit) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st(&reg, bit);
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

static inline int c_sla(uint8_t* reg) {
  st_c_rl(*reg);
  *reg = (uint8_t)(*reg << 1);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_sla_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rl(reg);
  reg = (uint8_t)(reg << 1);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}

static inline int c_sra(uint8_t* reg) {
  st_c_rr(*reg);
  uint8_t bt = (*reg >> 7) & 1;
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(bt << 7);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

static inline int c_sra_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rr(reg);
  uint8_t bt = (reg >> 7) & 1;
  reg = (uint8_t)(reg >> 1) | (uint8_t)(bt << 7);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 16;
}



static inline int c_swp(uint8_t* reg) {
  *reg = (uint8_t)(*reg >> 4) | (uint8_t)(*reg << 4);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 8;
}

static inline int c_swp_mem(uint16_t loc) {
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = (uint8_t)(reg >> 4) | (uint8_t)(reg << 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 16;
}

static inline int c_xor(uint8_t reg) {
  A ^= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}



static inline uint16_t gt_AF(void) { return (uint16_t)(((uint16_t)A) << 8) | (uint16_t)F; }

static inline void st_AF(uint16_t AF) {
  A = (uint8_t)(AF >> 8);
  F = (uint8_t)AF;
}

static inline uint16_t gt_BC(void) { return (uint16_t)(((uint16_t)B) << 8) | (uint16_t)C; }

static inline void st_BC(uint16_t BC) {
  B = (uint8_t)(BC >> 8);
  C = (uint8_t)BC;
}

static inline uint16_t gt_DE(void) { return (uint16_t)(((uint16_t)D) << 8) | (uint16_t)E; }

static inline void st_DE(uint16_t DE) {
  D = (uint8_t)(DE >> 8);
  E = (uint8_t)DE;
}

static inline uint16_t gt_HL(void) { return (uint16_t)(((uint16_t)H) << 8) | (uint16_t)L; }

static inline void st_HL(uint16_t HL) {
  H = (uint8_t)(HL >> 8);
  L = (uint8_t)HL;
}

// Print instruction to stdout (disassembly)
int disassemble(uint8_t instr, uint8_t prfx);

void do_intr(uint8_t intr);
