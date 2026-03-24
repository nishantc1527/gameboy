#include <stdint.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"

/* 0x40 LD B,B (+ ld_b_b_fired) */
uint8_t op_40(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->B;  // NOLINT
  cpu->ld_b_b_fired = true;
  return 4;
}
/* 0x41 LD B,C */
uint8_t op_41(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->C;
  return 4;
}
/* 0x42 LD B,D */
uint8_t op_42(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->D;
  return 4;
}
/* 0x43 LD B,E */
uint8_t op_43(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->E;
  return 4;
}
/* 0x44 LD B,H */
uint8_t op_44(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->H;
  return 4;
}
/* 0x45 LD B,L */
uint8_t op_45(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->L;
  return 4;
}
/* 0x46 LD B,(HL) */
uint8_t op_46(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->B = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x47 LD B,A */
uint8_t op_47(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->B = cpu->A;
  return 4;
}
/* 0x48 LD C,B */
uint8_t op_48(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->B;
  return 4;
}
/* 0x49 LD C,C */
uint8_t op_49(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->C;  // NOLINT
  return 4;
}
/* 0x4A LD C,D */
uint8_t op_4A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->D;
  return 4;
}
/* 0x4B LD C,E */
uint8_t op_4B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->E;
  return 4;
}
/* 0x4C LD C,H */
uint8_t op_4C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->H;
  return 4;
}
/* 0x4D LD C,L */
uint8_t op_4D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->L;
  return 4;
}
/* 0x4E LD C,(HL) */
uint8_t op_4E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->C = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x4F LD C,A */
uint8_t op_4F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->C = cpu->A;
  return 4;
}
/* 0x50 LD D,B */
uint8_t op_50(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->B;
  return 4;
}
/* 0x51 LD D,C */
uint8_t op_51(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->C;
  return 4;
}
/* 0x52 LD D,D */
uint8_t op_52(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->D;  // NOLINT
  return 4;
}
/* 0x53 LD D,E */
uint8_t op_53(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->E;
  return 4;
}
/* 0x54 LD D,H */
uint8_t op_54(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->H;
  return 4;
}
/* 0x55 LD D,L */
uint8_t op_55(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->L;
  return 4;
}
/* 0x56 LD D,(HL) */
uint8_t op_56(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->D = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x57 LD D,A */
uint8_t op_57(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->D = cpu->A;
  return 4;
}
/* 0x58 LD E,B */
uint8_t op_58(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->B;
  return 4;
}
/* 0x59 LD E,C */
uint8_t op_59(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->C;
  return 4;
}
/* 0x5A LD E,D */
uint8_t op_5A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->D;
  return 4;
}
/* 0x5B LD E,E */
uint8_t op_5B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->E;  // NOLINT
  return 4;
}
/* 0x5C LD E,H */
uint8_t op_5C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->H;
  return 4;
}
/* 0x5D LD E,L */
uint8_t op_5D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->L;
  return 4;
}
/* 0x5E LD E,(HL) */
uint8_t op_5E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->E = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x5F LD E,A */
uint8_t op_5F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->E = cpu->A;
  return 4;
}
/* 0x60 LD H,B */
uint8_t op_60(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->B;
  return 4;
}
/* 0x61 LD H,C */
uint8_t op_61(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->C;
  return 4;
}
/* 0x62 LD H,D */
uint8_t op_62(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->D;
  return 4;
}
/* 0x63 LD H,E */
uint8_t op_63(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->E;
  return 4;
}
/* 0x64 LD H,H */
uint8_t op_64(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->H;  // NOLINT
  return 4;
}
/* 0x65 LD H,L */
uint8_t op_65(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->L;
  return 4;
}
/* 0x66 LD H,(HL) */
uint8_t op_66(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->H = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x67 LD H,A */
uint8_t op_67(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->H = cpu->A;
  return 4;
}
/* 0x68 LD L,B */
uint8_t op_68(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->B;
  return 4;
}
/* 0x69 LD L,C */
uint8_t op_69(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->C;
  return 4;
}
/* 0x6A LD L,D */
uint8_t op_6A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->D;
  return 4;
}
/* 0x6B LD L,E */
uint8_t op_6B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->E;
  return 4;
}
/* 0x6C LD L,H */
uint8_t op_6C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->H;
  return 4;
}
/* 0x6D LD L,L */
uint8_t op_6D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->L;  // NOLINT
  return 4;
}
/* 0x6E LD L,(HL) */
uint8_t op_6E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->L = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x6F LD L,A */
uint8_t op_6F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->L = cpu->A;
  return 4;
}
/* 0x70 LD (HL),B */
uint8_t op_70(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->B);
  return 8;
}
/* 0x71 LD (HL),C */
uint8_t op_71(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->C);
  return 8;
}
/* 0x72 LD (HL),D */
uint8_t op_72(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->D);
  return 8;
}
/* 0x73 LD (HL),E */
uint8_t op_73(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->E);
  return 8;
}
/* 0x74 LD (HL),H */
uint8_t op_74(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->H);
  return 8;
}
/* 0x75 LD (HL),L */
uint8_t op_75(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->L);
  return 8;
}
/* 0x76 HALT */
uint8_t op_76(struct Cpu* cpu, struct Bus* bus) {
  if (!cpu->ime && (cpu->ie_reg & cpu->if_reg & IF_VALID_MASK)) {
    uint8_t op = bus_read(bus, cpu->PC);
    cpu_ops[op](cpu, bus);
  } else {
    cpu->halted = true;
  }
  return 4;
}
/* 0x77 LD (HL),A */
uint8_t op_77(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, gt_HL(cpu), cpu->A);
  return 8;
}
/* 0x78 LD A,B */
uint8_t op_78(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->B;
  return 4;
}
/* 0x79 LD A,C */
uint8_t op_79(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->C;
  return 4;
}
/* 0x7A LD A,D */
uint8_t op_7A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->D;
  return 4;
}
/* 0x7B LD A,E */
uint8_t op_7B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->E;
  return 4;
}
/* 0x7C LD A,H */
uint8_t op_7C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->H;
  return 4;
}
/* 0x7D LD A,L */
uint8_t op_7D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->L;
  return 4;
}
/* 0x7E LD A,(HL) */
uint8_t op_7E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, gt_HL(cpu));
  return 8;
}
/* 0x7F LD A,A */
uint8_t op_7F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->A = cpu->A;  // NOLINT
  return 4;
}
