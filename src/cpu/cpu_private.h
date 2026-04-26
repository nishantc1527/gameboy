#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/timer.h"
#include "gbemu/util.h"

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  bool halted;
  bool ime;
  bool ime_pending;
  uint8_t if_reg;
  uint8_t ie_reg;
  bool cgb_mode;
  bool ld_b_b_fired;  // Many tests exit by executing LD B, B
};

#define FLG_Z 7
#define FLG_N 6
#define FLG_H 5
#define FLG_C 4

#define IF_REG_ADDR 0xFF0F

#define INTR_DISPATCH_CYCLES 20

#define INTR_VEC_VBLANK 0x0040
#define INTR_VEC_LCD 0x0048
#define INTR_VEC_TIMER 0x0050
#define INTR_VEC_SERIAL 0x0058
#define INTR_VEC_JOYPAD 0x0060

#define CPU_BOOT_PC 0x0100
#define CPU_BOOT_SP 0xFFFE
#define CPU_BOOT_IF 0xE1
#define CPU_DMG_A 0x01
#define CPU_DMG_F_WITH_CHKSUM 0xB0
#define CPU_DMG_F_NO_CHKSUM 0x80
#define CPU_DMG_B 0x00
#define CPU_DMG_C 0x13
#define CPU_DMG_D 0x00
#define CPU_DMG_E 0xD8
#define CPU_DMG_H 0x01
#define CPU_DMG_L 0x4D

#define CPU_CGB_A 0x11
#define CPU_CGB_F 0x80
#define CPU_CGB_D 0xFF
#define CPU_CGB_E 0x56
#define CPU_CGB_H 0x00
#define CPU_CGB_L 0x0D

static inline void cpu_mem_tick(struct Cpu* cpu, struct Bus* bus,
                                uint8_t pre_cycles) {
  (void)cpu;
  timer_tick(bus->timer, pre_cycles, bus);
  timer_record_sub_cycles(bus->timer, pre_cycles);
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

static inline uint8_t fetch_byte(struct Cpu* cpu, struct Bus* bus) {
  return bus_read(bus, cpu->PC++);
}

static inline uint16_t fetch_word(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr1 = cpu->PC++;
  uint16_t addr2 = cpu->PC++;
  return (uint16_t)((uint16_t)bus_read(bus, addr2) << 8) |
         (uint16_t)bus_read(bus, addr1);
}

static inline void push(struct Cpu* cpu, struct Bus* bus, uint16_t val) {
  uint8_t val1 = (uint8_t)(val >> 8);
  uint8_t val2 = (uint8_t)val;
  bus_write(bus, (uint16_t)(cpu->SP - 1), val1);
  bus_write(bus, (uint16_t)(cpu->SP - 2), val2);
  cpu->SP -= 2;
}

static inline uint16_t pop(struct Cpu* cpu, struct Bus* bus) {
  uint16_t val1 = bus_read(bus, cpu->SP);
  uint16_t val2 = bus_read(bus, (uint16_t)(cpu->SP + 1));
  cpu->SP += 2;
  return val1 | (uint16_t)(val2 << 8);
}

static inline void set_zero_flag(struct Cpu* cpu, uint8_t var) {
  if (var == 0)
    set_flag(cpu, FLG_Z);
  else
    clear_flag(cpu, FLG_Z);
}
static inline void set_half_carry_add(struct Cpu* cpu, uint8_t var1,
                                      uint8_t var2) {
  if (((var1 & 0xF) + (var2 & 0xF)) & 0x10)
    set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void set_half_carry_add16(struct Cpu* cpu, uint16_t var1,
                                        uint16_t var2) {
  if (((var1 & 0xFFF) + (var2 & 0xFFF)) & 0x1000)
    set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void set_half_carry_sub(struct Cpu* cpu, uint8_t var1,
                                      uint8_t var2) {
  if (((var1 & 0x0F) - (var2 & 0x0F)) & 0x10)
    set_flag(cpu, FLG_H);
  else
    clear_flag(cpu, FLG_H);
}
static inline void set_carry_rotate_left(struct Cpu* cpu, uint8_t var) {
  if (var >> 7)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void set_carry_rotate_right(struct Cpu* cpu, uint8_t var) {
  if (var & 1)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void set_carry_add(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  uint16_t res = (uint16_t)var1 + (uint16_t)var2;
  if (res > 0xFF)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void set_carry_add16(struct Cpu* cpu, uint16_t var1,
                                   uint16_t var2) {
  int res = (int)var1 + (int)var2;
  if (res > 0xFFFF)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline void set_carry_sub(struct Cpu* cpu, uint8_t var1, uint8_t var2) {
  if (var1 < var2)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
}
static inline int c_add(struct Cpu* cpu, uint8_t reg) {
  set_half_carry_add(cpu, cpu->A, reg);
  set_carry_add(cpu, cpu->A, reg);
  cpu->A += reg;
  set_zero_flag(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  return 4;
}

static inline int c_adc(struct Cpu* cpu, uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (get_flag(cpu, FLG_C)) {
    set_half_carry_add(cpu, cpu->A, 1);
    set_carry_add(cpu, cpu->A, 1);
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
  set_zero_flag(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 4;
}

static inline int c_bit(struct Cpu* cpu, uint8_t reg, uint8_t bit) {
  if (get_bit(reg, bit))
    clear_flag(cpu, FLG_Z);
  else
    set_flag(cpu, FLG_Z);
  clear_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  return 8;
}

static inline int c_call(struct Cpu* cpu, struct Bus* bus, int flg) {
  if (flg) {
    push(cpu, bus, (uint16_t)(cpu->PC + 2));
    cpu->PC = fetch_word(cpu, bus);
    return 24;
  }
  fetch_word(cpu, bus);
  return 12;
}

static inline int c_cp(struct Cpu* cpu, uint8_t reg) {
  set_zero_flag(cpu, (uint8_t)(cpu->A - reg));
  set_flag(cpu, FLG_N);
  set_half_carry_sub(cpu, cpu->A, reg);
  set_carry_sub(cpu, cpu->A, reg);
  return 4;
}

static inline int c_cpl(struct Cpu* cpu, uint8_t* reg) {
  *reg = ~*reg;
  set_flag(cpu, FLG_N);
  set_flag(cpu, FLG_H);
  return 4;
}

static inline int c_dec(struct Cpu* cpu, uint8_t* reg) {
  set_half_carry_sub(cpu, *reg, 1);
  *reg = *reg - 1;
  set_zero_flag(cpu, *reg);
  set_flag(cpu, FLG_N);
  return 4;
}

static inline int c_dec_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 4);
  uint8_t reg = bus_read(bus, loc);
  set_half_carry_sub(cpu, reg, 1);
  reg = reg - 1;
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  set_flag(cpu, FLG_N);
  return 12;
}

static inline int c_inc(struct Cpu* cpu, uint8_t* reg) {
  set_half_carry_add(cpu, *reg, 1);
  *reg = *reg + 1;
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  return 4;
}

static inline int c_inc_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 4);
  uint8_t reg = bus_read(bus, loc);
  set_half_carry_add(cpu, reg, 1);
  reg = reg + 1;
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  return 12;
}

static inline int c_jp8(struct Cpu* cpu, struct Bus* bus, int flg) {
  int8_t offset = (int8_t)fetch_byte(cpu, bus);
  if (flg) {
    cpu->PC = (uint16_t)(cpu->PC + offset);
    return 12;
  }
  return 8;
}

static inline int c_jp16(struct Cpu* cpu, struct Bus* bus, int flg) {
  if (flg) {
    cpu->PC = fetch_word(cpu, bus);
    return 16;
  } else
    fetch_word(cpu, bus);
  return 12;
}

static inline int c_or(struct Cpu* cpu, uint8_t reg) {
  cpu->A |= reg;
  set_zero_flag(cpu, cpu->A);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 4;
}

static inline int c_res(uint8_t* reg, uint8_t bit) {
  *reg = (uint8_t)(*reg & ~(1 << bit));
  return 8;
}

static inline int c_res_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc,
                            int bit) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  reg = (uint8_t)(reg & ~(1 << bit));
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  return 16;
}

static inline int c_ret(struct Cpu* cpu, struct Bus* bus, int flg) {
  if (flg) {
    cpu->PC = pop(cpu, bus);
    return 20;
  }
  return 8;
}

static inline int c_rr(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = get_flag(cpu, FLG_C);
  set_carry_rotate_right(cpu, *reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rr_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  uint8_t carry = get_flag(cpu, FLG_C);
  set_carry_rotate_right(cpu, reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rrc(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = *reg & 1;
  set_carry_rotate_right(cpu, *reg);
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(carry << 7);
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rrc_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  uint8_t carry = reg & 1;
  set_carry_rotate_right(cpu, reg);
  reg = (uint8_t)(reg >> 1) | (uint8_t)(carry << 7);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rl(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = get_flag(cpu, FLG_C);
  set_carry_rotate_left(cpu, *reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rl_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  uint8_t carry = get_flag(cpu, FLG_C);
  set_carry_rotate_left(cpu, reg);
  reg = (uint8_t)(reg << 1) | carry;
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rlc(struct Cpu* cpu, uint8_t* reg) {
  uint8_t carry = (*reg >> 7) & 1;
  set_carry_rotate_left(cpu, *reg);
  *reg = (uint8_t)(*reg << 1) | carry;
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_rlc_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  uint8_t carry = (reg >> 7) & 1;
  set_carry_rotate_left(cpu, reg);
  reg = (uint8_t)(reg << 1) | carry;
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_rst(struct Cpu* cpu, struct Bus* bus, uint8_t loc) {
  push(cpu, bus, cpu->PC);
  cpu->PC = loc;
  return 16;
}

static inline int c_sub(struct Cpu* cpu, uint8_t reg) {
  set_half_carry_sub(cpu, cpu->A, reg);
  set_carry_sub(cpu, cpu->A, reg);
  cpu->A -= reg;
  set_zero_flag(cpu, cpu->A);
  set_flag(cpu, FLG_N);
  return 4;
}

static inline int c_sbc(struct Cpu* cpu, uint8_t reg) {
  int cy = 0;
  int hy = 0;
  if (get_flag(cpu, FLG_C)) {
    set_half_carry_sub(cpu, cpu->A, 1);
    set_carry_sub(cpu, cpu->A, 1);
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
  if (*reg & 1)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
  *reg = *reg >> 1;
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_srl_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  if (reg & 1)
    set_flag(cpu, FLG_C);
  else
    clear_flag(cpu, FLG_C);
  reg = reg >> 1;
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_set(uint8_t* reg, uint8_t bit) {
  set_bit(reg, bit);
  return 8;
}

static inline int c_set_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc,
                            uint8_t bit) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  set_bit(&reg, bit);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  return 16;
}

static inline int c_sla(struct Cpu* cpu, uint8_t* reg) {
  set_carry_rotate_left(cpu, *reg);
  *reg = (uint8_t)(*reg << 1);
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_sla_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  set_carry_rotate_left(cpu, reg);
  reg = (uint8_t)(reg << 1);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_sra(struct Cpu* cpu, uint8_t* reg) {
  set_carry_rotate_right(cpu, *reg);
  uint8_t bt = (*reg >> 7) & 1;
  *reg = (uint8_t)(*reg >> 1) | (uint8_t)(bt << 7);
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 8;
}

static inline int c_sra_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  set_carry_rotate_right(cpu, reg);
  uint8_t bt = (reg >> 7) & 1;
  reg = (uint8_t)(reg >> 1) | (uint8_t)(bt << 7);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  return 16;
}

static inline int c_swp(struct Cpu* cpu, uint8_t* reg) {
  *reg = (uint8_t)(*reg >> 4) | (uint8_t)(*reg << 4);
  set_zero_flag(cpu, *reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 8;
}

static inline int c_swp_mem(struct Cpu* cpu, struct Bus* bus, uint16_t loc) {
  cpu_mem_tick(cpu, bus, 8);
  uint8_t reg = bus_read(bus, loc);
  reg = (uint8_t)(reg >> 4) | (uint8_t)(reg << 4);
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, loc, reg);
  set_zero_flag(cpu, reg);
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  clear_flag(cpu, FLG_C);
  return 16;
}

static inline int c_xor(struct Cpu* cpu, uint8_t reg) {
  cpu->A ^= reg;
  set_zero_flag(cpu, cpu->A);
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

void disassemble(struct Cpu* cpu, struct Bus* bus, const uint16_t* watch_addrs,
                 uint8_t watch_count, uint64_t total_cycles);

int do_intr(struct Cpu* cpu, struct Bus* bus, uint8_t intr);
uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus);
