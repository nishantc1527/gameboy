#include <stdint.h>
#include <stdio.h>

#include "cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"

static uint8_t op_xx(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  fprintf(stderr, "UNIMPLEMENTED INSTRUCTION\n");
  return 0;
}
/* 0x00 NOP */
static uint8_t op_00(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 4;
}
/* 0x01 LD BC,d16 */
static uint8_t op_01(struct Cpu* cpu, struct Bus* bus) {
  st_BC(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x02 LD (BC),A */
static uint8_t op_02(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_BC(cpu), cpu->A);
  return 8;
}
/* 0x03 INC BC */
static uint8_t op_03(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_BC(cpu, gt_BC(cpu) + 1);
  return 8;
}
/* 0x04 INC B */
static uint8_t op_04(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->B);
}
/* 0x05 DEC B */
static uint8_t op_05(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->B);
}
/* 0x06 LD B,d8 */
static uint8_t op_06(struct Cpu* cpu, struct Bus* bus) {
  cpu->B = fetch_byte(cpu, bus);
  return 8;
}
/* 0x07 RLCA */
static uint8_t op_07(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rlc(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x08 LD (a16),SP */
static uint8_t op_08(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  bus_write(bus, addr, (uint8_t)(cpu->SP & 0xFF));
  bus_write(bus, (uint16_t)(addr + 1), (uint8_t)((cpu->SP >> 8) & 0xFF));
  return 20;
}
/* 0x09 ADD HL,BC */
static uint8_t op_09(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_BC(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_BC(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_BC(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x0A LD A,(BC) */
static uint8_t op_0A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_BC(cpu));
  return 8;
}
/* 0x0B DEC BC */
static uint8_t op_0B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_BC(cpu, gt_BC(cpu) - 1);
  return 8;
}
/* 0x0C INC C */
static uint8_t op_0C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->C);
}
/* 0x0D DEC C */
static uint8_t op_0D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->C);
}
/* 0x0E LD C,d8 */
static uint8_t op_0E(struct Cpu* cpu, struct Bus* bus) {
  cpu->C = fetch_byte(cpu, bus);
  return 8;
}
/* 0x0F RRCA */
static uint8_t op_0F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rrc(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x10 STOP */
static uint8_t op_10(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 4;
}
/* 0x11 LD DE,d16 */
static uint8_t op_11(struct Cpu* cpu, struct Bus* bus) {
  st_DE(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x12 LD (DE),A */
static uint8_t op_12(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_DE(cpu), cpu->A);
  return 8;
}
/* 0x13 INC DE */
static uint8_t op_13(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_DE(cpu, gt_DE(cpu) + 1);
  return 8;
}
/* 0x14 INC D */
static uint8_t op_14(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->D);
}
/* 0x15 DEC D */
static uint8_t op_15(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->D);
}
/* 0x16 LD D,d8 */
static uint8_t op_16(struct Cpu* cpu, struct Bus* bus) {
  cpu->D = fetch_byte(cpu, bus);
  return 8;
}
/* 0x17 RLA */
static uint8_t op_17(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rl(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x18 JR r8 */
static uint8_t op_18(struct Cpu* cpu, struct Bus* bus) {
  int8_t offset = (int8_t)fetch_byte(cpu, bus);
  cpu->PC = (uint16_t)(cpu->PC + offset);
  return 12;
}
/* 0x19 ADD HL,DE */
static uint8_t op_19(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_DE(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_DE(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_DE(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x1A LD A,(DE) */
static uint8_t op_1A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_DE(cpu));
  return 8;
}
/* 0x1B DEC DE */
static uint8_t op_1B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_DE(cpu, gt_DE(cpu) - 1);
  return 8;
}
/* 0x1C INC E */
static uint8_t op_1C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->E);
}
/* 0x1D DEC E */
static uint8_t op_1D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->E);
}
/* 0x1E LD E,d8 */
static uint8_t op_1E(struct Cpu* cpu, struct Bus* bus) {
  cpu->E = fetch_byte(cpu, bus);
  return 8;
}
/* 0x1F RRA */
static uint8_t op_1F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  c_rr(cpu, &cpu->A);
  clear_flag(cpu, FLG_Z);
  return 4;
}
/* 0x20 JR NZ,r8 */
static uint8_t op_20(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0x21 LD HL,d16 */
static uint8_t op_21(struct Cpu* cpu, struct Bus* bus) {
  st_HL(cpu, fetch_word(cpu, bus));
  return 12;
}
/* 0x22 LD (HL+),A */
static uint8_t op_22(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x23 INC HL */
static uint8_t op_23(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x24 INC H */
static uint8_t op_24(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->H);
}
/* 0x25 DEC H */
static uint8_t op_25(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->H);
}
/* 0x26 LD H,d8 */
static uint8_t op_26(struct Cpu* cpu, struct Bus* bus) {
  cpu->H = fetch_byte(cpu, bus);
  return 8;
}
/* 0x27 DAA */
static uint8_t op_27(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  if (!get_flag(cpu, FLG_N)) {
    if (get_flag(cpu, FLG_C) || cpu->A > 0x99) {
      cpu->A += 0x60;
      set_flag(cpu, FLG_C);
    }
    if (get_flag(cpu, FLG_H) || (cpu->A & 0x0f) > 0x09) {
      cpu->A += 0x06;
    }
  } else {
    if (get_flag(cpu, FLG_C)) cpu->A -= 0x60;
    if (get_flag(cpu, FLG_H)) cpu->A -= 0x06;
  }
  set_zero_flag(cpu, cpu->A);
  clear_flag(cpu, FLG_H);
  return 4;
}
/* 0x28 JR Z,r8 */
static uint8_t op_28(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0x29 ADD HL,HL */
static uint8_t op_29(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), gt_HL(cpu));
  set_carry_add16(cpu, gt_HL(cpu), gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) + gt_HL(cpu));
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x2A LD A,(HL+) */
static uint8_t op_2A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) + 1);
  return 8;
}
/* 0x2B DEC HL */
static uint8_t op_2B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x2C INC L */
static uint8_t op_2C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->L);
}
/* 0x2D DEC L */
static uint8_t op_2D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->L);
}
/* 0x2E LD L,d8 */
static uint8_t op_2E(struct Cpu* cpu, struct Bus* bus) {
  cpu->L = fetch_byte(cpu, bus);
  return 8;
}
/* 0x2F CPL */
static uint8_t op_2F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cpl(cpu, &cpu->A);
}
/* 0x30 JR NC,r8 */
static uint8_t op_30(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0x31 LD SP,d16 */
static uint8_t op_31(struct Cpu* cpu, struct Bus* bus) {
  cpu->SP = fetch_word(cpu, bus);
  return 12;
}
/* 0x32 LD (HL-),A */
static uint8_t op_32(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x33 INC SP */
static uint8_t op_33(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  ++cpu->SP;
  return 8;
}
/* 0x34 INC (HL) */
static uint8_t op_34(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_inc_mem(cpu, bus, gt_HL(cpu));
}
/* 0x35 DEC (HL) */
static uint8_t op_35(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_dec_mem(cpu, bus, gt_HL(cpu));
}
/* 0x36 LD (HL),d8 */
static uint8_t op_36(struct Cpu* cpu, struct Bus* bus) {
  uint8_t imm = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  bus_write(bus, gt_HL(cpu), imm);
  return 12;
}
/* 0x37 SCF */
static uint8_t op_37(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  set_flag(cpu, FLG_C);
  return 4;
}
/* 0x38 JR C,r8 */
static uint8_t op_38(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp8(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0x39 ADD HL,SP */
static uint8_t op_39(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  set_half_carry_add16(cpu, gt_HL(cpu), cpu->SP);
  set_carry_add16(cpu, gt_HL(cpu), cpu->SP);
  st_HL(cpu, gt_HL(cpu) + cpu->SP);
  clear_flag(cpu, FLG_N);
  return 8;
}
/* 0x3A LD A,(HL-) */
static uint8_t op_3A(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  st_HL(cpu, gt_HL(cpu) - 1);
  return 8;
}
/* 0x3B DEC SP */
static uint8_t op_3B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  --cpu->SP;
  return 8;
}
/* 0x3C INC A */
static uint8_t op_3C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_inc(cpu, &cpu->A);
}
/* 0x3D DEC A */
static uint8_t op_3D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_dec(cpu, &cpu->A);
}
/* 0x3E LD A,d8 */
static uint8_t op_3E(struct Cpu* cpu, struct Bus* bus) {
  cpu->A = fetch_byte(cpu, bus);
  return 8;
}
/* 0x3F CCF */
static uint8_t op_3F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  clear_flag(cpu, FLG_N);
  clear_flag(cpu, FLG_H);
  if (get_flag(cpu, FLG_C))
    clear_flag(cpu, FLG_C);
  else
    set_flag(cpu, FLG_C);
  return 4;
}
/* 0x40 LD B,B (+ ld_b_b_fired) */
static uint8_t op_40(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->B;  // NOLINT
  cpu->ld_b_b_fired = true;
  return 4;
}
/* 0x41 LD B,C */
static uint8_t op_41(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->C;
  return 4;
}
/* 0x42 LD B,D */
static uint8_t op_42(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->D;
  return 4;
}
/* 0x43 LD B,E */
static uint8_t op_43(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->E;
  return 4;
}
/* 0x44 LD B,H */
static uint8_t op_44(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->H;
  return 4;
}
/* 0x45 LD B,L */
static uint8_t op_45(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->L;
  return 4;
}
/* 0x46 LD B,(HL) */
static uint8_t op_46(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->B = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x47 LD B,A */
static uint8_t op_47(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->A;
  return 4;
}
/* 0x48 LD C,B */
static uint8_t op_48(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->B;
  return 4;
}
/* 0x49 LD C,C */
static uint8_t op_49(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->C;  // NOLINT
  return 4;
}
/* 0x4A LD C,D */
static uint8_t op_4A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->D;
  return 4;
}
/* 0x4B LD C,E */
static uint8_t op_4B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->E;
  return 4;
}
/* 0x4C LD C,H */
static uint8_t op_4C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->H;
  return 4;
}
/* 0x4D LD C,L */
static uint8_t op_4D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->L;
  return 4;
}
/* 0x4E LD C,(HL) */
static uint8_t op_4E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->C = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x4F LD C,A */
static uint8_t op_4F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->A;
  return 4;
}
/* 0x50 LD D,B */
static uint8_t op_50(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->B;
  return 4;
}
/* 0x51 LD D,C */
static uint8_t op_51(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->C;
  return 4;
}
/* 0x52 LD D,D */
static uint8_t op_52(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->D;  // NOLINT
  return 4;
}
/* 0x53 LD D,E */
static uint8_t op_53(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->E;
  return 4;
}
/* 0x54 LD D,H */
static uint8_t op_54(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->H;
  return 4;
}
/* 0x55 LD D,L */
static uint8_t op_55(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->L;
  return 4;
}
/* 0x56 LD D,(HL) */
static uint8_t op_56(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->D = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x57 LD D,A */
static uint8_t op_57(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->A;
  return 4;
}
/* 0x58 LD E,B */
static uint8_t op_58(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->B;
  return 4;
}
/* 0x59 LD E,C */
static uint8_t op_59(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->C;
  return 4;
}
/* 0x5A LD E,D */
static uint8_t op_5A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->D;
  return 4;
}
/* 0x5B LD E,E */
static uint8_t op_5B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->E;  // NOLINT
  return 4;
}
/* 0x5C LD E,H */
static uint8_t op_5C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->H;
  return 4;
}
/* 0x5D LD E,L */
static uint8_t op_5D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->L;
  return 4;
}
/* 0x5E LD E,(HL) */
static uint8_t op_5E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->E = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x5F LD E,A */
static uint8_t op_5F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->A;
  return 4;
}
/* 0x60 LD H,B */
static uint8_t op_60(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->B;
  return 4;
}
/* 0x61 LD H,C */
static uint8_t op_61(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->C;
  return 4;
}
/* 0x62 LD H,D */
static uint8_t op_62(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->D;
  return 4;
}
/* 0x63 LD H,E */
static uint8_t op_63(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->E;
  return 4;
}
/* 0x64 LD H,H */
static uint8_t op_64(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->H;  // NOLINT
  return 4;
}
/* 0x65 LD H,L */
static uint8_t op_65(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->L;
  return 4;
}
/* 0x66 LD H,(HL) */
static uint8_t op_66(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->H = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x67 LD H,A */
static uint8_t op_67(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->A;
  return 4;
}
/* 0x68 LD L,B */
static uint8_t op_68(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->B;
  return 4;
}
/* 0x69 LD L,C */
static uint8_t op_69(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->C;
  return 4;
}
/* 0x6A LD L,D */
static uint8_t op_6A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->D;
  return 4;
}
/* 0x6B LD L,E */
static uint8_t op_6B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->E;
  return 4;
}
/* 0x6C LD L,H */
static uint8_t op_6C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->H;
  return 4;
}
/* 0x6D LD L,L */
static uint8_t op_6D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->L;  // NOLINT
  return 4;
}
/* 0x6E LD L,(HL) */
static uint8_t op_6E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->L = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x6F LD L,A */
static uint8_t op_6F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->A;
  return 4;
}
/* 0x70 LD (HL),B */
static uint8_t op_70(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->B);
  return 8;
}
/* 0x71 LD (HL),C */
static uint8_t op_71(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->C);
  return 8;
}
/* 0x72 LD (HL),D */
static uint8_t op_72(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->D);
  return 8;
}
/* 0x73 LD (HL),E */
static uint8_t op_73(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->E);
  return 8;
}
/* 0x74 LD (HL),H */
static uint8_t op_74(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->H);
  return 8;
}
/* 0x75 LD (HL),L */
static uint8_t op_75(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->L);
  return 8;
}
/* 0x76 HALT */
static uint8_t op_76(struct Cpu* cpu, struct Bus* bus) {
  if (!cpu->ime && (cpu->ie_reg & cpu->if_reg & IF_VALID_MASK)) {
    uint8_t op = bus_read(bus, cpu->PC);
    cpu_ops[op](cpu, bus);
  } else {
    cpu->halted = true;
  }
  return 4;
}
/* 0x77 LD (HL),A */
static uint8_t op_77(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  return 8;
}
/* 0x78 LD A,B */
static uint8_t op_78(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->B;
  return 4;
}
/* 0x79 LD A,C */
static uint8_t op_79(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->C;
  return 4;
}
/* 0x7A LD A,D */
static uint8_t op_7A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->D;
  return 4;
}
/* 0x7B LD A,E */
static uint8_t op_7B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->E;
  return 4;
}
/* 0x7C LD A,H */
static uint8_t op_7C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->H;
  return 4;
}
/* 0x7D LD A,L */
static uint8_t op_7D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->L;
  return 4;
}
/* 0x7E LD A,(HL) */
static uint8_t op_7E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x7F LD A,A */
static uint8_t op_7F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->A;  // NOLINT
  return 4;
}
/* 0x80 ADD A,B */
static uint8_t op_80(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->B);
}
/* 0x81 ADD A,C */
static uint8_t op_81(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->C);
}
/* 0x82 ADD A,D */
static uint8_t op_82(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->D);
}
/* 0x83 ADD A,E */
static uint8_t op_83(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->E);
}
/* 0x84 ADD A,H */
static uint8_t op_84(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->H);
}
/* 0x85 ADD A,L */
static uint8_t op_85(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->L);
}
/* 0x86 ADD A,(HL) */
static uint8_t op_86(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_add(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x87 ADD A,A */
static uint8_t op_87(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->A);
}
/* 0x88 ADC A,B */
static uint8_t op_88(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->B);
}
/* 0x89 ADC A,C */
static uint8_t op_89(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->C);
}
/* 0x8A ADC A,D */
static uint8_t op_8A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->D);
}
/* 0x8B ADC A,E */
static uint8_t op_8B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->E);
}
/* 0x8C ADC A,H */
static uint8_t op_8C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->H);
}
/* 0x8D ADC A,L */
static uint8_t op_8D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->L);
}
/* 0x8E ADC A,(HL) */
static uint8_t op_8E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_adc(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x8F ADC A,A */
static uint8_t op_8F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->A);
}
/* 0x90 SUB B */
static uint8_t op_90(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->B);
}
/* 0x91 SUB C */
static uint8_t op_91(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->C);
}
/* 0x92 SUB D */
static uint8_t op_92(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->D);
}
/* 0x93 SUB E */
static uint8_t op_93(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->E);
}
/* 0x94 SUB H */
static uint8_t op_94(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->H);
}
/* 0x95 SUB L */
static uint8_t op_95(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->L);
}
/* 0x96 SUB (HL) */
static uint8_t op_96(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_sub(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x97 SUB A */
static uint8_t op_97(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->A);
}
/* 0x98 SBC A,B */
static uint8_t op_98(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->B);
}
/* 0x99 SBC A,C */
static uint8_t op_99(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->C);
}
/* 0x9A SBC A,D */
static uint8_t op_9A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->D);
}
/* 0x9B SBC A,E */
static uint8_t op_9B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->E);
}
/* 0x9C SBC A,H */
static uint8_t op_9C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->H);
}
/* 0x9D SBC A,L */
static uint8_t op_9D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->L);
}
/* 0x9E SBC A,(HL) */
static uint8_t op_9E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_sbc(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x9F SBC A,A */
static uint8_t op_9F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->A);
}
/* 0xA0 AND B */
static uint8_t op_A0(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->B);
}
/* 0xA1 AND C */
static uint8_t op_A1(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->C);
}
/* 0xA2 AND D */
static uint8_t op_A2(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->D);
}
/* 0xA3 AND E */
static uint8_t op_A3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->E);
}
/* 0xA4 AND H */
static uint8_t op_A4(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->H);
}
/* 0xA5 AND L */
static uint8_t op_A5(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->L);
}
/* 0xA6 AND (HL) */
static uint8_t op_A6(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_and(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xA7 AND A */
static uint8_t op_A7(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->A);
}
/* 0xA8 XOR B */
static uint8_t op_A8(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->B);
}
/* 0xA9 XOR C */
static uint8_t op_A9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->C);
}
/* 0xAA XOR D */
static uint8_t op_AA(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->D);
}
/* 0xAB XOR E */
static uint8_t op_AB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->E);
}
/* 0xAC XOR H */
static uint8_t op_AC(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->H);
}
/* 0xAD XOR L */
static uint8_t op_AD(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->L);
}
/* 0xAE XOR (HL) */
static uint8_t op_AE(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_xor(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xAF XOR A */
static uint8_t op_AF(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->A);
}
/* 0xB0 OR B */
static uint8_t op_B0(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->B);
}
/* 0xB1 OR C */
static uint8_t op_B1(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->C);
}
/* 0xB2 OR D */
static uint8_t op_B2(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->D);
}
/* 0xB3 OR E */
static uint8_t op_B3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->E);
}
/* 0xB4 OR H */
static uint8_t op_B4(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->H);
}
/* 0xB5 OR L */
static uint8_t op_B5(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->L);
}
/* 0xB6 OR (HL) */
static uint8_t op_B6(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_or(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xB7 OR A */
static uint8_t op_B7(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->A);
}
/* 0xB8 CP B */
static uint8_t op_B8(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->B);
}
/* 0xB9 CP C */
static uint8_t op_B9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->C);
}
/* 0xBA CP D */
static uint8_t op_BA(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->D);
}
/* 0xBB CP E */
static uint8_t op_BB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->E);
}
/* 0xBC CP H */
static uint8_t op_BC(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->H);
}
/* 0xBD CP L */
static uint8_t op_BD(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->L);
}
/* 0xBE CP (HL) */
static uint8_t op_BE(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_cp(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xBF CP A */
static uint8_t op_BF(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->A);
}
/* 0xC0 RET NZ */
static uint8_t op_C0(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC1 POP BC */
static uint8_t op_C1(struct Cpu* cpu, struct Bus* bus) {
  st_BC(cpu, pop(cpu, bus));
  return 12;
}
/* 0xC2 JP NZ,a16 */
static uint8_t op_C2(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC3 JP a16 */
static uint8_t op_C3(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1);
}
/* 0xC4 CALL NZ,a16 */
static uint8_t op_C4(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC5 PUSH BC */
static uint8_t op_C5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_BC(cpu));
  return 16;
}
/* 0xC6 ADD A,d8 */
static uint8_t op_C6(struct Cpu* cpu, struct Bus* bus) {
  c_add(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xC7 RST 00H */
static uint8_t op_C7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0000);
}
/* 0xC8 RET Z */
static uint8_t op_C8(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xC9 RET */
static uint8_t op_C9(struct Cpu* cpu, struct Bus* bus) {
  c_ret(cpu, bus, 1);
  return 16;
}
/* 0xCA JP Z,a16 */
static uint8_t op_CA(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xCB is handled by cpu_step, should never be called */
static uint8_t op_CB(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 0; /* should never be reached */
}
/* 0xCC CALL Z,a16 */
static uint8_t op_CC(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xCD CALL a16 */
static uint8_t op_CD(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1);
}
/* 0xCE ADC A,d8 */
static uint8_t op_CE(struct Cpu* cpu, struct Bus* bus) {
  c_adc(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xCF RST 08H */
static uint8_t op_CF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0008);
}
/* 0xD0 RET NC */
static uint8_t op_D0(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD1 POP DE */
static uint8_t op_D1(struct Cpu* cpu, struct Bus* bus) {
  st_DE(cpu, pop(cpu, bus));
  return 12;
}
/* 0xD2 JP NC,a16 */
static uint8_t op_D2(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD3 illegal */
/* 0xD4 CALL NC,a16 */
static uint8_t op_D4(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD5 PUSH DE */
static uint8_t op_D5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_DE(cpu));
  return 16;
}
/* 0xD6 SUB d8 */
static uint8_t op_D6(struct Cpu* cpu, struct Bus* bus) {
  c_sub(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xD7 RST 10H */
static uint8_t op_D7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0010);
}
/* 0xD8 RET C */
static uint8_t op_D8(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xD9 RETI */
static uint8_t op_D9(struct Cpu* cpu, struct Bus* bus) {
  cpu->ime = true;
  c_ret(cpu, bus, 1);
  return 16;
}
/* 0xDA JP C,a16 */
static uint8_t op_DA(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xDB illegal */
/* 0xDC CALL C,a16 */
static uint8_t op_DC(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xDD illegal */
/* 0xDE SBC A,d8 */
static uint8_t op_DE(struct Cpu* cpu, struct Bus* bus) {
  c_sbc(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xDF RST 18H */
static uint8_t op_DF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0018);
}
/* 0xE0 LDH (a8),A */
static uint8_t op_E0(struct Cpu* cpu, struct Bus* bus) {
  uint8_t n = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  bus_write(bus, 0xFF00 + (uint16_t)n, cpu->A);
  return 12;
}
/* 0xE1 POP HL */
static uint8_t op_E1(struct Cpu* cpu, struct Bus* bus) {
  st_HL(cpu, pop(cpu, bus));
  return 12;
}
/* 0xE2 LD (C),A */
static uint8_t op_E2(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, 0xFF00 + cpu->C, cpu->A);
  return 8;
}
/* 0xE3 illegal */
/* 0xE4 illegal */
/* 0xE5 PUSH HL */
static uint8_t op_E5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_HL(cpu));
  return 16;
}
/* 0xE6 AND d8 */
static uint8_t op_E6(struct Cpu* cpu, struct Bus* bus) {
  c_and(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xE7 RST 20H */
static uint8_t op_E7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0020);
}
/* 0xE8 ADD SP,r8 */
static uint8_t op_E8(struct Cpu* cpu, struct Bus* bus) {
  uint8_t add = fetch_byte(cpu, bus);
  set_half_carry_add(cpu, (uint8_t)cpu->SP, add);
  set_carry_add(cpu, (uint8_t)cpu->SP, add);
  cpu->SP = (uint16_t)(cpu->SP + (int8_t)add);
  clear_flag(cpu, FLG_Z);
  clear_flag(cpu, FLG_N);
  return 16;
}
/* 0xE9 JP (HL) */
static uint8_t op_E9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->PC = gt_HL(cpu);
  return 4;
}
/* 0xEA LD (a16),A */
static uint8_t op_EA(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  cpu_mem_tick(cpu, bus, 12);
  bus_write(bus, addr, cpu->A);
  return 16;
}
/* 0xEB illegal */
/* 0xEC illegal */
/* 0xED illegal */
/* 0xEE XOR d8 */
static uint8_t op_EE(struct Cpu* cpu, struct Bus* bus) {
  c_xor(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xEF RST 28H */
static uint8_t op_EF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0028);
}
/* 0xF0 LDH A,(a8) */
static uint8_t op_F0(struct Cpu* cpu, struct Bus* bus) {
  uint8_t n = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  cpu->A = bus_read(bus, 0xFF00 + (uint16_t)n);
  return 12;
}
/* 0xF1 POP AF */
static uint8_t op_F1(struct Cpu* cpu, struct Bus* bus) {
  st_AF(cpu, pop(cpu, bus) & 0xFFF0);
  return 12;
}
/* 0xF2 LD A,(C) */
static uint8_t op_F2(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, 0xFF00 + cpu->C);
  return 8;
}
/* 0xF3 DI */
static uint8_t op_F3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->ime = false;
  return 4;
}
/* 0xF4 illegal */
/* 0xF5 PUSH AF */
static uint8_t op_F5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_AF(cpu));
  return 16;
}
/* 0xF6 OR d8 */
static uint8_t op_F6(struct Cpu* cpu, struct Bus* bus) {
  c_or(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xF7 RST 30H */
static uint8_t op_F7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0030);
}
/* 0xF8 LD HL,SP+r8 */
static uint8_t op_F8(struct Cpu* cpu, struct Bus* bus) {
  uint8_t nxt = fetch_byte(cpu, bus);
  uint16_t add = (uint16_t)(cpu->SP + (int8_t)nxt);
  st_HL(cpu, add);
  clear_flag(cpu, FLG_Z);
  clear_flag(cpu, FLG_N);
  set_half_carry_add(cpu, (uint8_t)cpu->SP, nxt);
  set_carry_add(cpu, (uint8_t)cpu->SP, nxt);
  return 12;
}
/* 0xF9 LD SP,HL */
static uint8_t op_F9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->SP = gt_HL(cpu);
  return 8;
}
/* 0xFA LD A,(a16) */
static uint8_t op_FA(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  cpu_mem_tick(cpu, bus, 12);
  cpu->A = bus_read(bus, addr);
  return 16;
}
/* 0xFB EI */
static uint8_t op_FB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->ime_pending = true;
  return 4;
}
/* 0xFC illegal */
/* 0xFD illegal */
/* 0xFE CP d8 */
static uint8_t op_FE(struct Cpu* cpu, struct Bus* bus) {
  c_cp(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xFF RST 38H */
static uint8_t op_FF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0038);
}

/* ========== CB TABLE ========== */

/* CB 0x00-0x07: RLC r */
static uint8_t cb_00(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->B);
}
static uint8_t cb_01(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->C);
}
static uint8_t cb_02(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->D);
}
static uint8_t cb_03(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->E);
}
static uint8_t cb_04(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->H);
}
static uint8_t cb_05(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->L);
}
static uint8_t cb_06(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rlc_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_07(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->A);
}
/* CB 0x08-0x0F: RRC r */
static uint8_t cb_08(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->B);
}
static uint8_t cb_09(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->C);
}
static uint8_t cb_0A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->D);
}
static uint8_t cb_0B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->E);
}
static uint8_t cb_0C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->H);
}
static uint8_t cb_0D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->L);
}
static uint8_t cb_0E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rrc_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_0F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->A);
}
/* CB 0x10-0x17: RL r */
static uint8_t cb_10(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->B);
}
static uint8_t cb_11(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->C);
}
static uint8_t cb_12(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->D);
}
static uint8_t cb_13(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->E);
}
static uint8_t cb_14(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->H);
}
static uint8_t cb_15(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->L);
}
static uint8_t cb_16(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rl_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_17(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->A);
}
/* CB 0x18-0x1F: RR r */
static uint8_t cb_18(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->B);
}
static uint8_t cb_19(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->C);
}
static uint8_t cb_1A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->D);
}
static uint8_t cb_1B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->E);
}
static uint8_t cb_1C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->H);
}
static uint8_t cb_1D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->L);
}
static uint8_t cb_1E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rr_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_1F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->A);
}
/* CB 0x20-0x27: SLA r */
static uint8_t cb_20(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->B);
}
static uint8_t cb_21(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->C);
}
static uint8_t cb_22(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->D);
}
static uint8_t cb_23(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->E);
}
static uint8_t cb_24(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->H);
}
static uint8_t cb_25(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->L);
}
static uint8_t cb_26(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_sla_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_27(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->A);
}
/* CB 0x28-0x2F: SRA r */
static uint8_t cb_28(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->B);
}
static uint8_t cb_29(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->C);
}
static uint8_t cb_2A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->D);
}
static uint8_t cb_2B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->E);
}
static uint8_t cb_2C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->H);
}
static uint8_t cb_2D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->L);
}
static uint8_t cb_2E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_sra_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_2F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->A);
}
/* CB 0x30-0x37: SWAP r */
static uint8_t cb_30(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->B);
}
static uint8_t cb_31(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->C);
}
static uint8_t cb_32(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->D);
}
static uint8_t cb_33(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->E);
}
static uint8_t cb_34(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->H);
}
static uint8_t cb_35(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->L);
}
static uint8_t cb_36(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_swp_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_37(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->A);
}
/* CB 0x38-0x3F: SRL r */
static uint8_t cb_38(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->B);
}
static uint8_t cb_39(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->C);
}
static uint8_t cb_3A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->D);
}
static uint8_t cb_3B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->E);
}
static uint8_t cb_3C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->H);
}
static uint8_t cb_3D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->L);
}
static uint8_t cb_3E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_srl_mem(cpu, bus, gt_HL(cpu));
}
static uint8_t cb_3F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->A);
}

/* CB 0x40-0x7F: BIT b,r — note: BIT (HL) has mem_tick */
#define CB_BIT_REG(HEX, BIT, REG)                             \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    (void)bus;                                                \
    return (uint8_t)c_bit(cpu, cpu->REG, BIT);                \
  }
#define CB_BIT_HL(HEX, BIT)                                   \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    cpu_mem_tick(cpu, bus, 8);                                    \
    c_bit(cpu, bus_read(bus, gt_HL(cpu)), BIT);               \
    return 12;                                                \
  }

CB_BIT_REG(40, 0, B)
CB_BIT_REG(41, 0, C)
CB_BIT_REG(42, 0, D)
CB_BIT_REG(43, 0, E)
CB_BIT_REG(44, 0, H)
CB_BIT_REG(45, 0, L)
CB_BIT_HL(46, 0)
CB_BIT_REG(47, 0, A)
CB_BIT_REG(48, 1, B)
CB_BIT_REG(49, 1, C)
CB_BIT_REG(4A, 1, D)
CB_BIT_REG(4B, 1, E)
CB_BIT_REG(4C, 1, H)
CB_BIT_REG(4D, 1, L)
CB_BIT_HL(4E, 1)
CB_BIT_REG(4F, 1, A)
CB_BIT_REG(50, 2, B)
CB_BIT_REG(51, 2, C)
CB_BIT_REG(52, 2, D)
CB_BIT_REG(53, 2, E)
CB_BIT_REG(54, 2, H)
CB_BIT_REG(55, 2, L)
CB_BIT_HL(56, 2)
CB_BIT_REG(57, 2, A)
CB_BIT_REG(58, 3, B)
CB_BIT_REG(59, 3, C)
CB_BIT_REG(5A, 3, D)
CB_BIT_REG(5B, 3, E)
CB_BIT_REG(5C, 3, H)
CB_BIT_REG(5D, 3, L)
CB_BIT_HL(5E, 3)
CB_BIT_REG(5F, 3, A)
CB_BIT_REG(60, 4, B)
CB_BIT_REG(61, 4, C)
CB_BIT_REG(62, 4, D)
CB_BIT_REG(63, 4, E)
CB_BIT_REG(64, 4, H)
CB_BIT_REG(65, 4, L)
CB_BIT_HL(66, 4)
CB_BIT_REG(67, 4, A)
CB_BIT_REG(68, 5, B)
CB_BIT_REG(69, 5, C)
CB_BIT_REG(6A, 5, D)
CB_BIT_REG(6B, 5, E)
CB_BIT_REG(6C, 5, H)
CB_BIT_REG(6D, 5, L)
CB_BIT_HL(6E, 5)
CB_BIT_REG(6F, 5, A)
CB_BIT_REG(70, 6, B)
CB_BIT_REG(71, 6, C)
CB_BIT_REG(72, 6, D)
CB_BIT_REG(73, 6, E)
CB_BIT_REG(74, 6, H)
CB_BIT_REG(75, 6, L)
CB_BIT_HL(76, 6)
CB_BIT_REG(77, 6, A)
CB_BIT_REG(78, 7, B)
CB_BIT_REG(79, 7, C)
CB_BIT_REG(7A, 7, D)
CB_BIT_REG(7B, 7, E)
CB_BIT_REG(7C, 7, H)
CB_BIT_REG(7D, 7, L)
CB_BIT_HL(7E, 7)
CB_BIT_REG(7F, 7, A)

/* CB 0x80-0xBF: RES b,r */
#define CB_RES_REG(HEX, BIT, REG)                             \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    (void)bus;                                                \
    return (uint8_t)c_res(&cpu->REG, BIT);                    \
  }
#define CB_RES_HL(HEX, BIT)                                   \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    return (uint8_t)c_res_mem(cpu, bus, gt_HL(cpu), BIT);     \
  }

CB_RES_REG(80, 0, B)
CB_RES_REG(81, 0, C)
CB_RES_REG(82, 0, D)
CB_RES_REG(83, 0, E)
CB_RES_REG(84, 0, H)
CB_RES_REG(85, 0, L)
CB_RES_HL(86, 0)
CB_RES_REG(87, 0, A)
CB_RES_REG(88, 1, B)
CB_RES_REG(89, 1, C)
CB_RES_REG(8A, 1, D)
CB_RES_REG(8B, 1, E)
CB_RES_REG(8C, 1, H)
CB_RES_REG(8D, 1, L)
CB_RES_HL(8E, 1)
CB_RES_REG(8F, 1, A)
CB_RES_REG(90, 2, B)
CB_RES_REG(91, 2, C)
CB_RES_REG(92, 2, D)
CB_RES_REG(93, 2, E)
CB_RES_REG(94, 2, H)
CB_RES_REG(95, 2, L)
CB_RES_HL(96, 2)
CB_RES_REG(97, 2, A)
CB_RES_REG(98, 3, B)
CB_RES_REG(99, 3, C)
CB_RES_REG(9A, 3, D)
CB_RES_REG(9B, 3, E)
CB_RES_REG(9C, 3, H)
CB_RES_REG(9D, 3, L)
CB_RES_HL(9E, 3)
CB_RES_REG(9F, 3, A)
CB_RES_REG(A0, 4, B)
CB_RES_REG(A1, 4, C)
CB_RES_REG(A2, 4, D)
CB_RES_REG(A3, 4, E)
CB_RES_REG(A4, 4, H)
CB_RES_REG(A5, 4, L)
CB_RES_HL(A6, 4)
CB_RES_REG(A7, 4, A)
CB_RES_REG(A8, 5, B)
CB_RES_REG(A9, 5, C)
CB_RES_REG(AA, 5, D)
CB_RES_REG(AB, 5, E)
CB_RES_REG(AC, 5, H)
CB_RES_REG(AD, 5, L)
CB_RES_HL(AE, 5)
CB_RES_REG(AF, 5, A)
CB_RES_REG(B0, 6, B)
CB_RES_REG(B1, 6, C)
CB_RES_REG(B2, 6, D)
CB_RES_REG(B3, 6, E)
CB_RES_REG(B4, 6, H)
CB_RES_REG(B5, 6, L)
CB_RES_HL(B6, 6)
CB_RES_REG(B7, 6, A)
CB_RES_REG(B8, 7, B)
CB_RES_REG(B9, 7, C)
CB_RES_REG(BA, 7, D)
CB_RES_REG(BB, 7, E)
CB_RES_REG(BC, 7, H)
CB_RES_REG(BD, 7, L)
CB_RES_HL(BE, 7)
CB_RES_REG(BF, 7, A)

/* CB 0xC0-0xFF: SET b,r */
#define CB_SET_REG(HEX, BIT, REG)                             \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    (void)bus;                                                \
    return (uint8_t)c_set(&cpu->REG, BIT);                    \
  }
#define CB_SET_HL(HEX, BIT)                                   \
  static uint8_t cb_##HEX(struct Cpu* cpu, struct Bus* bus) { \
    return (uint8_t)c_set_mem(cpu, bus, gt_HL(cpu), BIT);     \
  }

CB_SET_REG(C0, 0, B)
CB_SET_REG(C1, 0, C)
CB_SET_REG(C2, 0, D)
CB_SET_REG(C3, 0, E)
CB_SET_REG(C4, 0, H)
CB_SET_REG(C5, 0, L)
CB_SET_HL(C6, 0)
CB_SET_REG(C7, 0, A)
CB_SET_REG(C8, 1, B)
CB_SET_REG(C9, 1, C)
CB_SET_REG(CA, 1, D)
CB_SET_REG(CB, 1, E)
CB_SET_REG(CC, 1, H)
CB_SET_REG(CD, 1, L)
CB_SET_HL(CE, 1)
CB_SET_REG(CF, 1, A)
CB_SET_REG(D0, 2, B)
CB_SET_REG(D1, 2, C)
CB_SET_REG(D2, 2, D)
CB_SET_REG(D3, 2, E)
CB_SET_REG(D4, 2, H)
CB_SET_REG(D5, 2, L)
CB_SET_HL(D6, 2)
CB_SET_REG(D7, 2, A)
CB_SET_REG(D8, 3, B)
CB_SET_REG(D9, 3, C)
CB_SET_REG(DA, 3, D)
CB_SET_REG(DB, 3, E)
CB_SET_REG(DC, 3, H)
CB_SET_REG(DD, 3, L)
CB_SET_HL(DE, 3)
CB_SET_REG(DF, 3, A)
CB_SET_REG(E0, 4, B)
CB_SET_REG(E1, 4, C)
CB_SET_REG(E2, 4, D)
CB_SET_REG(E3, 4, E)
CB_SET_REG(E4, 4, H)
CB_SET_REG(E5, 4, L)
CB_SET_HL(E6, 4)
CB_SET_REG(E7, 4, A)
CB_SET_REG(E8, 5, B)
CB_SET_REG(E9, 5, C)
CB_SET_REG(EA, 5, D)
CB_SET_REG(EB, 5, E)
CB_SET_REG(EC, 5, H)
CB_SET_REG(ED, 5, L)
CB_SET_HL(EE, 5)
CB_SET_REG(EF, 5, A)
CB_SET_REG(F0, 6, B)
CB_SET_REG(F1, 6, C)
CB_SET_REG(F2, 6, D)
CB_SET_REG(F3, 6, E)
CB_SET_REG(F4, 6, H)
CB_SET_REG(F5, 6, L)
CB_SET_HL(F6, 6)
CB_SET_REG(F7, 6, A)
CB_SET_REG(F8, 7, B)
CB_SET_REG(F9, 7, C)
CB_SET_REG(FA, 7, D)
CB_SET_REG(FB, 7, E)
CB_SET_REG(FC, 7, H)
CB_SET_REG(FD, 7, L)
CB_SET_HL(FE, 7)
CB_SET_REG(FF, 7, A)

const CpuOp cpu_ops[256] = {
    op_00, op_01, op_02, op_03, op_04, op_05, op_06, op_07, op_08, op_09, op_0A,
    op_0B, op_0C, op_0D, op_0E, op_0F, op_10, op_11, op_12, op_13, op_14, op_15,
    op_16, op_17, op_18, op_19, op_1A, op_1B, op_1C, op_1D, op_1E, op_1F, op_20,
    op_21, op_22, op_23, op_24, op_25, op_26, op_27, op_28, op_29, op_2A, op_2B,
    op_2C, op_2D, op_2E, op_2F, op_30, op_31, op_32, op_33, op_34, op_35, op_36,
    op_37, op_38, op_39, op_3A, op_3B, op_3C, op_3D, op_3E, op_3F, op_40, op_41,
    op_42, op_43, op_44, op_45, op_46, op_47, op_48, op_49, op_4A, op_4B, op_4C,
    op_4D, op_4E, op_4F, op_50, op_51, op_52, op_53, op_54, op_55, op_56, op_57,
    op_58, op_59, op_5A, op_5B, op_5C, op_5D, op_5E, op_5F, op_60, op_61, op_62,
    op_63, op_64, op_65, op_66, op_67, op_68, op_69, op_6A, op_6B, op_6C, op_6D,
    op_6E, op_6F, op_70, op_71, op_72, op_73, op_74, op_75, op_76, op_77, op_78,
    op_79, op_7A, op_7B, op_7C, op_7D, op_7E, op_7F, op_80, op_81, op_82, op_83,
    op_84, op_85, op_86, op_87, op_88, op_89, op_8A, op_8B, op_8C, op_8D, op_8E,
    op_8F, op_90, op_91, op_92, op_93, op_94, op_95, op_96, op_97, op_98, op_99,
    op_9A, op_9B, op_9C, op_9D, op_9E, op_9F, op_A0, op_A1, op_A2, op_A3, op_A4,
    op_A5, op_A6, op_A7, op_A8, op_A9, op_AA, op_AB, op_AC, op_AD, op_AE, op_AF,
    op_B0, op_B1, op_B2, op_B3, op_B4, op_B5, op_B6, op_B7, op_B8, op_B9, op_BA,
    op_BB, op_BC, op_BD, op_BE, op_BF, op_C0, op_C1, op_C2, op_C3, op_C4, op_C5,
    op_C6, op_C7, op_C8, op_C9, op_CA, op_CB, op_CC, op_CD, op_CE, op_CF, op_D0,
    op_D1, op_D2, op_xx, op_D4, op_D5, op_D6, op_D7, op_D8, op_D9, op_DA, op_xx,
    op_DC, op_xx, op_DE, op_DF, op_E0, op_E1, op_E2, op_xx, op_xx, op_E5, op_E6,
    op_E7, op_E8, op_E9, op_EA, op_xx, op_xx, op_xx, op_EE, op_EF, op_F0, op_F1,
    op_F2, op_F3, op_xx, op_F5, op_F6, op_F7, op_F8, op_F9, op_FA, op_FB, op_xx,
    op_xx, op_FE, op_FF,
};

const CpuOp cpu_cb_ops[256] = {
    cb_00, cb_01, cb_02, cb_03, cb_04, cb_05, cb_06, cb_07, cb_08, cb_09, cb_0A,
    cb_0B, cb_0C, cb_0D, cb_0E, cb_0F, cb_10, cb_11, cb_12, cb_13, cb_14, cb_15,
    cb_16, cb_17, cb_18, cb_19, cb_1A, cb_1B, cb_1C, cb_1D, cb_1E, cb_1F, cb_20,
    cb_21, cb_22, cb_23, cb_24, cb_25, cb_26, cb_27, cb_28, cb_29, cb_2A, cb_2B,
    cb_2C, cb_2D, cb_2E, cb_2F, cb_30, cb_31, cb_32, cb_33, cb_34, cb_35, cb_36,
    cb_37, cb_38, cb_39, cb_3A, cb_3B, cb_3C, cb_3D, cb_3E, cb_3F, cb_40, cb_41,
    cb_42, cb_43, cb_44, cb_45, cb_46, cb_47, cb_48, cb_49, cb_4A, cb_4B, cb_4C,
    cb_4D, cb_4E, cb_4F, cb_50, cb_51, cb_52, cb_53, cb_54, cb_55, cb_56, cb_57,
    cb_58, cb_59, cb_5A, cb_5B, cb_5C, cb_5D, cb_5E, cb_5F, cb_60, cb_61, cb_62,
    cb_63, cb_64, cb_65, cb_66, cb_67, cb_68, cb_69, cb_6A, cb_6B, cb_6C, cb_6D,
    cb_6E, cb_6F, cb_70, cb_71, cb_72, cb_73, cb_74, cb_75, cb_76, cb_77, cb_78,
    cb_79, cb_7A, cb_7B, cb_7C, cb_7D, cb_7E, cb_7F, cb_80, cb_81, cb_82, cb_83,
    cb_84, cb_85, cb_86, cb_87, cb_88, cb_89, cb_8A, cb_8B, cb_8C, cb_8D, cb_8E,
    cb_8F, cb_90, cb_91, cb_92, cb_93, cb_94, cb_95, cb_96, cb_97, cb_98, cb_99,
    cb_9A, cb_9B, cb_9C, cb_9D, cb_9E, cb_9F, cb_A0, cb_A1, cb_A2, cb_A3, cb_A4,
    cb_A5, cb_A6, cb_A7, cb_A8, cb_A9, cb_AA, cb_AB, cb_AC, cb_AD, cb_AE, cb_AF,
    cb_B0, cb_B1, cb_B2, cb_B3, cb_B4, cb_B5, cb_B6, cb_B7, cb_B8, cb_B9, cb_BA,
    cb_BB, cb_BC, cb_BD, cb_BE, cb_BF, cb_C0, cb_C1, cb_C2, cb_C3, cb_C4, cb_C5,
    cb_C6, cb_C7, cb_C8, cb_C9, cb_CA, cb_CB, cb_CC, cb_CD, cb_CE, cb_CF, cb_D0,
    cb_D1, cb_D2, cb_D3, cb_D4, cb_D5, cb_D6, cb_D7, cb_D8, cb_D9, cb_DA, cb_DB,
    cb_DC, cb_DD, cb_DE, cb_DF, cb_E0, cb_E1, cb_E2, cb_E3, cb_E4, cb_E5, cb_E6,
    cb_E7, cb_E8, cb_E9, cb_EA, cb_EB, cb_EC, cb_ED, cb_EE, cb_EF, cb_F0, cb_F1,
    cb_F2, cb_F3, cb_F4, cb_F5, cb_F6, cb_F7, cb_F8, cb_F9, cb_FA, cb_FB, cb_FC,
    cb_FD, cb_FE, cb_FF,
};
