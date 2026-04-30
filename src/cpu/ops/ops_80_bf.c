#include <stdint.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"
#include "ops_private.h"

/* 0x80 ADD A,B */
uint8_t op_80(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->B);
}
/* 0x81 ADD A,C */
uint8_t op_81(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->C);
}
/* 0x82 ADD A,D */
uint8_t op_82(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->D);
}
/* 0x83 ADD A,E */
uint8_t op_83(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->E);
}
/* 0x84 ADD A,H */
uint8_t op_84(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->H);
}
/* 0x85 ADD A,L */
uint8_t op_85(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->L);
}
/* 0x86 ADD A,(HL) */
uint8_t op_86(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_add(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x87 ADD A,A */
uint8_t op_87(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_add(cpu, cpu->A);
}
/* 0x88 ADC A,B */
uint8_t op_88(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->B);
}
/* 0x89 ADC A,C */
uint8_t op_89(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->C);
}
/* 0x8A ADC A,D */
uint8_t op_8A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->D);
}
/* 0x8B ADC A,E */
uint8_t op_8B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->E);
}
/* 0x8C ADC A,H */
uint8_t op_8C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->H);
}
/* 0x8D ADC A,L */
uint8_t op_8D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->L);
}
/* 0x8E ADC A,(HL) */
uint8_t op_8E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_adc(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x8F ADC A,A */
uint8_t op_8F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_adc(cpu, cpu->A);
}
/* 0x90 SUB B */
uint8_t op_90(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->B);
}
/* 0x91 SUB C */
uint8_t op_91(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->C);
}
/* 0x92 SUB D */
uint8_t op_92(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->D);
}
/* 0x93 SUB E */
uint8_t op_93(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->E);
}
/* 0x94 SUB H */
uint8_t op_94(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->H);
}
/* 0x95 SUB L */
uint8_t op_95(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->L);
}
/* 0x96 SUB (HL) */
uint8_t op_96(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_sub(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x97 SUB A */
uint8_t op_97(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sub(cpu, cpu->A);
}
/* 0x98 SBC A,B */
uint8_t op_98(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->B);
}
/* 0x99 SBC A,C */
uint8_t op_99(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->C);
}
/* 0x9A SBC A,D */
uint8_t op_9A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->D);
}
/* 0x9B SBC A,E */
uint8_t op_9B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->E);
}
/* 0x9C SBC A,H */
uint8_t op_9C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->H);
}
/* 0x9D SBC A,L */
uint8_t op_9D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->L);
}
/* 0x9E SBC A,(HL) */
uint8_t op_9E(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_sbc(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0x9F SBC A,A */
uint8_t op_9F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sbc(cpu, cpu->A);
}
/* 0xA0 AND B */
uint8_t op_A0(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->B);
}
/* 0xA1 AND C */
uint8_t op_A1(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->C);
}
/* 0xA2 AND D */
uint8_t op_A2(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->D);
}
/* 0xA3 AND E */
uint8_t op_A3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->E);
}
/* 0xA4 AND H */
uint8_t op_A4(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->H);
}
/* 0xA5 AND L */
uint8_t op_A5(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->L);
}
/* 0xA6 AND (HL) */
uint8_t op_A6(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_and(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xA7 AND A */
uint8_t op_A7(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_and(cpu, cpu->A);
}
/* 0xA8 XOR B */
uint8_t op_A8(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->B);
}
/* 0xA9 XOR C */
uint8_t op_A9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->C);
}
/* 0xAA XOR D */
uint8_t op_AA(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->D);
}
/* 0xAB XOR E */
uint8_t op_AB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->E);
}
/* 0xAC XOR H */
uint8_t op_AC(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->H);
}
/* 0xAD XOR L */
uint8_t op_AD(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->L);
}
/* 0xAE XOR (HL) */
uint8_t op_AE(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_xor(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xAF XOR A */
uint8_t op_AF(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_xor(cpu, cpu->A);
}
/* 0xB0 OR B */
uint8_t op_B0(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->B);
}
/* 0xB1 OR C */
uint8_t op_B1(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->C);
}
/* 0xB2 OR D */
uint8_t op_B2(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->D);
}
/* 0xB3 OR E */
uint8_t op_B3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->E);
}
/* 0xB4 OR H */
uint8_t op_B4(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->H);
}
/* 0xB5 OR L */
uint8_t op_B5(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->L);
}
/* 0xB6 OR (HL) */
uint8_t op_B6(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_or(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xB7 OR A */
uint8_t op_B7(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_or(cpu, cpu->A);
}
/* 0xB8 CP B */
uint8_t op_B8(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->B);
}
/* 0xB9 CP C */
uint8_t op_B9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->C);
}
/* 0xBA CP D */
uint8_t op_BA(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->D);
}
/* 0xBB CP E */
uint8_t op_BB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->E);
}
/* 0xBC CP H */
uint8_t op_BC(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->H);
}
/* 0xBD CP L */
uint8_t op_BD(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->L);
}
/* 0xBE CP (HL) */
uint8_t op_BE(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  c_cp(cpu, bus_read(bus, gt_HL(cpu)));
  return 8;
}
/* 0xBF CP A */
uint8_t op_BF(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_cp(cpu, cpu->A);
}
