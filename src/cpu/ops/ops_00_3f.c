#include <stdint.h>
#include <stdio.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"
#include "ops_private.h"

uint8_t op_xx(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  (void)fprintf(stderr, "UNIMPLEMENTED INSTRUCTION\n");
  return 0;
}
/* 0x00 NOP */
uint8_t op_00(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 4;
}
/* 0x01 LD BC,d16 */
uint8_t op_01(struct Cpu* cpu, struct Bus* bus) {
  st_BC(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x02 LD (BC),A */
uint8_t op_02(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_BC(cpu), cpu->A);
  return 8;
}
/* 0x03 INC BC */
uint8_t op_03(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_BC(cpu, gt_BC(cpu) + 1);
  return 8;
}
/* 0x04 INC B */
uint8_t op_04(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->B);
}
/* 0x05 DEC B */
uint8_t op_05(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->B);
}
/* 0x06 LD B,d8 */
uint8_t op_06(struct Cpu* cpu, struct Bus* bus) {
  cpu->B = fetch_byte(cpu, bus);
  return 8;
}
/* 0x07 RLCA */
uint8_t op_07(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rlc(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x08 LD (a16),SP */
uint8_t op_08(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  bus_write(bus, addr, (uint8_t)((unsigned)cpu->SP & 0xFFU));
  bus_write(bus, (uint16_t)(addr + 1),
            (uint8_t)(((unsigned)cpu->SP >> 8U) & 0xFFU));
  return 20;
}
/* 0x09 ADD HL,BC */
uint8_t op_09(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_BC(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_BC(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_BC(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x0A LD A,(BC) */
uint8_t op_0A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_BC(cpu));
  return 8;
}
/* 0x0B DEC BC */
uint8_t op_0B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_BC(cpu, gt_BC(cpu) - 1);
  return 8;
}
/* 0x0C INC C */
uint8_t op_0C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->C);
}
/* 0x0D DEC C */
uint8_t op_0D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->C);
}
/* 0x0E LD C,d8 */
uint8_t op_0E(struct Cpu* cpu, struct Bus* bus) {
  cpu->C = fetch_byte(cpu, bus);
  return 8;
}
/* 0x0F RRCA */
uint8_t op_0F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rrc(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x10 STOP */
uint8_t op_10(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 4;
}
/* 0x11 LD DE,d16 */
uint8_t op_11(struct Cpu* cpu, struct Bus* bus) {
  st_DE(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x12 LD (DE),A */
uint8_t op_12(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_DE(cpu), cpu->A);
  return 8;
}
/* 0x13 INC DE */
uint8_t op_13(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_DE(cpu, gt_DE(cpu) + 1);
  return 8;
}
/* 0x14 INC D */
uint8_t op_14(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->D);
}
/* 0x15 DEC D */
uint8_t op_15(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->D);
}
/* 0x16 LD D,d8 */
uint8_t op_16(struct Cpu* cpu, struct Bus* bus) {
  cpu->D = fetch_byte(cpu, bus);
  return 8;
}
/* 0x17 RLA */
uint8_t op_17(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rl(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x18 JR r8 */
uint8_t op_18(struct Cpu* cpu, struct Bus* bus) {
  int8_t offset = (int8_t)fetch_byte(cpu, bus);
  cpu->PC = (uint16_t)(cpu->PC + offset);
  return 12;
}
/* 0x19 ADD HL,DE */
uint8_t op_19(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_DE(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_DE(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_DE(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x1A LD A,(DE) */
uint8_t op_1A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_DE(cpu));
  return 8;
}
/* 0x1B DEC DE */
uint8_t op_1B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_DE(cpu, gt_DE(cpu) - 1);
  return 8;
}
/* 0x1C INC E */
uint8_t op_1C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->E);
}
/* 0x1D DEC E */
uint8_t op_1D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->E);
}
/* 0x1E LD E,d8 */
uint8_t op_1E(struct Cpu* cpu, struct Bus* bus) {
  cpu->E = fetch_byte(cpu, bus);
  return 8;
}
/* 0x1F RRA */
uint8_t op_1F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rr(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x20 JR NZ,r8 */
uint8_t op_20(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0x21 LD HL,d16 */
uint8_t op_21(struct Cpu* cpu, struct Bus* bus) {
  st_HL(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x22 LD (HL+),A */
uint8_t op_22(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x23 INC HL */
uint8_t op_23(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x24 INC H */
uint8_t op_24(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->H);
}
/* 0x25 DEC H */
uint8_t op_25(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->H);
}
/* 0x26 LD H,d8 */
uint8_t op_26(struct Cpu* cpu, struct Bus* bus) {
  cpu->H = fetch_byte(cpu, bus);
  return 8;
}
/* 0x27 DAA */
uint8_t op_27(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  if (!get_flag(cpu, FLG_N)) {
    if (get_flag(cpu, FLG_C) || cpu->A > 0x99) {
      cpu->A += 0x60;
      set_flag(cpu, FLG_C);
    }
    if (get_flag(cpu, FLG_H) || ((unsigned)cpu->A & 0x0fU) > 0x09U) {
      cpu->A += 0x06;
    }
  } else {
    if (get_flag(cpu, FLG_C)) {
      cpu->A -= 0x60;
    }
    if (get_flag(cpu, FLG_H)) {
      cpu->A -= 0x06;
    }
  }
  set_zero_flag(cpu, cpu->A);
  clear_flag(cpu, FLG_H);
  return 4;
}
/* 0x28 JR Z,r8 */
uint8_t op_28(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0x29 ADD HL,HL */
uint8_t op_29(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_HL(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_HL(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x2A LD A,(HL+) */
uint8_t op_2A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x2B DEC HL */
uint8_t op_2B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x2C INC L */
uint8_t op_2C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->L);
}
/* 0x2D DEC L */
uint8_t op_2D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->L);
}
/* 0x2E LD L,d8 */
uint8_t op_2E(struct Cpu* cpu, struct Bus* bus) {
  cpu->L = fetch_byte(cpu, bus);
  return 8;
}
/* 0x2F CPL */
uint8_t op_2F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cpl(cpu, &cpu->A);
}
/* 0x30 JR NC,r8 */
uint8_t op_30(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0x31 LD SP,d16 */
uint8_t op_31(struct Cpu* cpu, struct Bus* bus) {
  cpu->SP = fetch_word(cpu, bus);
  return 12;
}
/* 0x32 LD (HL-),A */
uint8_t op_32(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x33 INC SP */
uint8_t op_33(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  ++cpu->SP;
  return 8;
}
/* 0x34 INC (HL) */
uint8_t op_34(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_inc_mem(cpu, bus, gt_HL(cpu));
}
/* 0x35 DEC (HL) */
uint8_t op_35(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_dec_mem(cpu, bus, gt_HL(cpu));
}
/* 0x36 LD (HL),d8 */
uint8_t op_36(struct Cpu* cpu, struct Bus* bus) {
  uint8_t imm = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  bus_write(bus, gt_HL(cpu), imm);
  return 12;
}
/* 0x37 SCF */
uint8_t op_37(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  set_flag(cpu, FLG_C);
  return 4;
}
/* 0x38 JR C,r8 */
uint8_t op_38(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0x39 ADD HL,SP */
uint8_t op_39(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), cpu->SP);
  set_carry_add16(cpu, gt_HL(cpu), cpu->SP);
  st_HL(cpu, gt_HL(cpu) + cpu->SP);
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x3A LD A,(HL-) */
uint8_t op_3A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x3B DEC SP */
uint8_t op_3B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  --cpu->SP;
  return 8;
}
/* 0x3C INC A */
uint8_t op_3C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->A);
}
/* 0x3D DEC A */
uint8_t op_3D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->A);
}
/* 0x3E LD A,d8 */
uint8_t op_3E(struct Cpu* cpu, struct Bus* bus) {
  cpu->A = fetch_byte(cpu, bus);
  return 8;
}
/* 0x3F CCF */
uint8_t op_3F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  if (get_flag(cpu, FLG_C)) {
    clear_flag(cpu, FLG_C);
  } else {
    set_flag(cpu, FLG_C);
  }
  return 4;
}
