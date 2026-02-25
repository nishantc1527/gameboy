#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

#define FLG_Z 7
#define FLG_N 6
#define FLG_H 5
#define FLG_C 4

#define INTR_VBLANK 0
#define INTR_LCD 1
#define INTR_TIMER 2
#define INTR_SERIAL 3
#define INTR_JOYPAD 4

static inline void mem_tick(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                                   uint8_t pre_cycles) {
  update_timer(cpu, mmu, apu, pre_cycles);
  cpu->cyc_ext = (uint8_t)(cpu->cyc_ext + pre_cycles);
}

static inline uint8_t get_flag(struct Cpu* cpu, uint8_t flg) {
  return get_bit(cpu->F, flg);
}
static inline void set_flag(struct Cpu* cpu, uint8_t flg) {
  set_bit(&cpu->F, flg);
}
static inline void clear_flag(struct Cpu* cpu, uint8_t flg) {
  clear_bit(&cpu->F, flg);
}

static inline uint8_t rd8(struct Cpu* cpu, Mmu* mmu) {
  return mmu_r_mem(mmu, cpu->PC++);
}

static inline uint16_t rd16(struct Cpu* cpu, Mmu* mmu) {
  uint16_t addr1 = cpu->PC++;
  uint16_t addr2 = cpu->PC++;
  return (uint16_t)((uint16_t)mmu_r_mem(mmu, addr2) << 8) |
         (uint16_t)mmu_r_mem(mmu, addr1);
}

static inline void push(struct Cpu* cpu, Mmu* mmu, uint16_t val) {
  uint8_t val1 = (uint8_t)(val >> 8);
  uint8_t val2 = (uint8_t)val;
  mmu_w_mem(mmu, cpu->SP - 1, val1);
  mmu_w_mem(mmu, cpu->SP - 2, val2);
  cpu->SP -= 2;
}

static inline uint16_t pop(struct Cpu* cpu, Mmu* mmu) {
  uint16_t val1 = mmu_r_mem(mmu, cpu->SP);
  uint16_t val2 = mmu_r_mem(mmu, cpu->SP + 1);
  cpu->SP += 2;
  return val1 | (uint16_t)(val2 << 8);
}

static inline uint16_t pk(struct Cpu* cpu, Mmu* mmu) {
  uint16_t val = pop(cpu, mmu);
  push(cpu, mmu, val);
  return val;
}

static inline void st_z(struct Cpu* cpu, uint8_t var) {
  if (var == 0) set_flag(cpu, FLG_Z);
  else
    clear_flag(cpu, FLG_Z);
}
static inline void st_h_add(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  if (((var1 & 0xF) + (var2 & 0xF)) & 0x10) set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void st_h_add16(struct Cpu* cpu, uint16_t var1, uint16_t var2) {
  if (((var1 & 0xFFF) + (var2 & 0xFFF)) & 0x1000) set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void st_h_sub(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  if (((var1 & 0x0F) - (var2 & 0x0F)) & 0x10) set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void st_c_rl(struct Cpu* cpu, uint8_t var) {
  if (var >> 7) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void st_c_rr(struct Cpu* cpu, uint8_t var) {
  if (var & 1) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void st_c_add(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  uint16_t res = (uint16_t)var1 + (uint16_t)var2;
  if (res > 0xFF) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void st_c_add16(struct Cpu* cpu, uint16_t var1, uint16_t var2) {
  int res = (int)var1 + (int)var2;
  if (res > 0xFFFF) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void st_c_sub(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  if (var1 < var2) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void req_intr(Mmu* mmu, uint8_t intr) {
  uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
  set_bit(&val, intr);
  mmu_w_mem_raw(mmu, 0xFF0F, val);
}

static inline int c_add(struct Cpu* cpu, uint8_t reg) {
  st_h_add(cpu, cpu->A, reg);
  st_c_add(cpu, cpu->A, reg);
  cpu->A += reg;
  st_z(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  return 4;
}

static inline int c_adc(struct Cpu* cpu, uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (get_flag(cpu, FLG_C)) {
    st_h_add(cpu, cpu->A, 1);
    st_c_add(cpu, cpu->A, 1);
    cy = get_flag(cpu, FLG_C);
    hy = get_flag(cpu, FLG_H);
    cpu->A++;
  }
  c_add(cpu, reg);
  if (cy) set_flag(cpu, FLG_C);
  if (hy) set_flag(cpu, FLG_H);
  return 4;
}

static inline int c_and(struct Cpu* cpu, uint8_t reg) {
  cpu->A &= reg;
  st_z(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 4;
}

static inline int c_bit(struct Cpu* cpu, uint8_t reg, uint8_t bit) {
  if (get_bit(reg, bit)) clear_flag(cpu, FLG_Z);
  else
    set_flag(cpu, FLG_Z);
  clear_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  return 8;
}

static inline int c_call(struct Cpu* cpu, Mmu* mmu, int flg) {
  if (flg) {
    push(cpu, mmu, cpu->PC + 2);
    cpu->PC = rd16(cpu, mmu);
    return 24;
  }
  rd16(cpu, mmu);
  return 12;
}

static inline int c_cp(struct Cpu* cpu, uint8_t reg) {
  st_z(cpu, cpu->A - reg);
  set_flag(cpu, FLG_N);
  st_h_sub(cpu, cpu->A, reg);
  st_c_sub(cpu, cpu->A, reg);
  return 4;
}

static inline int c_cpl(struct Cpu* cpu, uint8_t* reg) {
  *reg = ~*reg;
  set_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  return 4;
}

static inline int c_dec(struct Cpu* cpu, uint8_t* reg) {
  st_h_sub(cpu, *reg, 1);
  *reg = *reg - 1;
  st_z(cpu, *reg);
  set_flag(cpu, FLG_N);
  return 4;
}

static inline int c_dec_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 4);
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_sub(cpu, reg, 1);
  reg = reg - 1;
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  set_flag(cpu, FLG_N);
  return 12;
}

static inline int c_inc(struct Cpu* cpu, uint8_t* reg) {
  st_h_add(cpu, *reg, 1);
  *reg = *reg + 1;
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  return 4;
}

static inline int c_inc_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 4);
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_h_add(cpu, reg, 1);
  reg = reg + 1;
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  return 12;
}

static inline int c_jp8(struct Cpu* cpu, Mmu* mmu, int flg) {
  int8_t offset = (int8_t)rd8(cpu, mmu);
  if (flg) {
    cpu->PC = (uint16_t)(cpu->PC + offset);
    return 12;
  }
  return 8;
}

static inline int c_jp16(struct Cpu* cpu, Mmu* mmu, int flg) {
  if (flg) {
    cpu->PC = rd16(cpu, mmu);
    return 16;
  } else
    rd16(cpu, mmu);
  return 12;
}

static inline int c_or(struct Cpu* cpu, uint8_t reg) {
  cpu->A |= reg;
  st_z(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 4;
}

static inline int c_res(uint8_t* reg, uint8_t bit) {
  *reg = (uint8_t)(*reg & ~(1 << bit));
  return 8;
}

static inline int c_res_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc, int bit) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = (uint8_t)(reg & ~(1 << bit));
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

static inline int c_ret(struct Cpu* cpu, Mmu* mmu, int flg) {
  if (flg) {
    cpu->PC = pop(cpu, mmu);
    return 20;
  }
  return 8;
}

static inline int c_rr(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = get_flag(cpu, FLG_C);
  st_c_rr(cpu, *reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rr_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                           uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = get_flag(cpu, FLG_C);
  st_c_rr(cpu, reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rrc(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = *reg & 1;
  st_c_rr(cpu, *reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rrc_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = reg & 1;
  st_c_rr(cpu, reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rl(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = get_flag(cpu, FLG_C);
  st_c_rl(cpu, *reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rl_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                           uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = get_flag(cpu, FLG_C);
  st_c_rl(cpu, reg);
  reg = (uint8_t)(reg << 1) | carry;
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rlc(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = (*reg >> 7) & 1;
  st_c_rl(cpu, *reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rlc_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  uint8_t carry = (reg >> 7) & 1;
  st_c_rl(cpu, reg);
  reg = (uint8_t)(reg << 1) | carry;
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rst(struct Cpu* cpu, Mmu* mmu, uint8_t loc) {
  push(cpu, mmu, cpu->PC);
  cpu->PC = loc;
  return 16;
}

static inline int c_sub(struct Cpu* cpu, uint8_t reg) {
  st_h_sub(cpu, cpu->A, reg);
  st_c_sub(cpu, cpu->A, reg);
  cpu->A -= reg;
  st_z(cpu, cpu->A);
  set_flag(cpu, FLG_N);
  return 4;
}

static inline int c_sbc(struct Cpu* cpu, uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (get_flag(cpu, FLG_C)) {
    st_h_sub(cpu, cpu->A, 1);
    st_c_sub(cpu, cpu->A, 1);
    cy = get_flag(cpu, FLG_C);
    hy = get_flag(cpu, FLG_H);
    cpu->A--;
  }
  c_sub(cpu, reg);
  if (cy) set_flag(cpu, FLG_C);
  if (hy) set_flag(cpu, FLG_H);
  return 4;
}

static inline int c_srl(struct Cpu* cpu, uint8_t* reg) {
  if (*reg & 1) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
  *reg = *reg >> 1;
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_srl_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  if (reg & 1) set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
  reg = reg >> 1;
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_set(uint8_t* reg, uint8_t bit) {
  set_bit(reg, bit);
  return 8;
}

static inline int c_set_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc, uint8_t bit) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  set_bit(&reg, bit);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  return 16;
}

static inline int c_sla(struct Cpu* cpu, uint8_t* reg) {
  st_c_rl(cpu, *reg);
  *reg = (uint8_t)(*reg << 1);
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_sla_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rl(cpu, reg);
  reg = (uint8_t)(reg << 1);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_sra(struct Cpu* cpu, uint8_t* reg) {
  st_c_rr(cpu, *reg);
  uint8_t bt = (*reg >> 7) & 1;
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(bt << 7);
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_sra_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  st_c_rr(cpu, reg);
  uint8_t bt = (reg >> 7) & 1;
  reg = (uint8_t)(reg >> 1) | (uint8_t)(bt << 7);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_swp(struct Cpu* cpu, uint8_t* reg) {
  *reg = (uint8_t)(*reg >> 4) | (uint8_t)(*reg << 4);
  st_z(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 8;
}

static inline int c_swp_mem(struct Cpu* cpu, Mmu* mmu, struct Apu* apu,
                            uint16_t loc) {
  mem_tick(cpu, mmu, apu, 8);
  uint8_t reg = mmu_r_mem(mmu, loc);
  reg = (uint8_t)(reg >> 4) | (uint8_t)(reg << 4);
  mem_tick(cpu, mmu, apu, 4);
  mmu_w_mem(mmu, loc, reg);
  st_z(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 16;
}

static inline int c_xor(struct Cpu* cpu, uint8_t reg) {
  cpu->A ^= reg;
  st_z(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 4;
}

static inline uint16_t gt_AF(struct Cpu* cpu) {
  return (uint16_t)(((uint16_t)cpu->A) << 8) | (uint16_t)cpu->F;
}

static inline void st_AF(struct Cpu* cpu, uint16_t AF) {
  cpu->A = (uint8_t)(AF >> 8);
  cpu->F = (uint8_t)AF;
}

static inline uint16_t gt_BC(struct Cpu* cpu) {
  return (uint16_t)(((uint16_t)cpu->B) << 8) | (uint16_t)cpu->C;
}

static inline void st_BC(struct Cpu* cpu, uint16_t BC) {
  cpu->B = (uint8_t)(BC >> 8);
  cpu->C = (uint8_t)BC;
}

static inline uint16_t gt_DE(struct Cpu* cpu) {
  return (uint16_t)(((uint16_t)cpu->D) << 8) | (uint16_t)cpu->E;
}

static inline void st_DE(struct Cpu* cpu, uint16_t DE) {
  cpu->D = (uint8_t)(DE >> 8);
  cpu->E = (uint8_t)DE;
}

static inline uint16_t gt_HL(struct Cpu* cpu) {
  return (uint16_t)(((uint16_t)cpu->H) << 8) | (uint16_t)cpu->L;
}

static inline void st_HL(struct Cpu* cpu, uint16_t HL) {
  cpu->H = (uint8_t)(HL >> 8);
  cpu->L = (uint8_t)HL;
}

// Print instruction to stdout (disassembly)
int disassemble(struct Cpu* cpu, Mmu* mmu, uint8_t instr, uint8_t prfx,
                const uint16_t* watch_addrs, uint8_t watch_count);

void do_intr(struct Cpu* cpu, Mmu* mmu, uint8_t intr);
