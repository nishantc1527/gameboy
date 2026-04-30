#include <stdint.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"
#include "ops_private.h"

/* 0xC0 RET NZ */
uint8_t op_C0(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC1 POP BC */
uint8_t op_C1(struct Cpu* cpu, struct Bus* bus) {
  st_BC(cpu, pop(cpu, bus));
  return 12;
}
/* 0xC2 JP NZ,a16 */
uint8_t op_C2(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC3 JP a16 */
uint8_t op_C3(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1);
}
/* 0xC4 CALL NZ,a16 */
uint8_t op_C4(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1 - get_flag(cpu, FLG_Z));
}
/* 0xC5 PUSH BC */
uint8_t op_C5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_BC(cpu));
  return 16;
}
/* 0xC6 ADD A,d8 */
uint8_t op_C6(struct Cpu* cpu, struct Bus* bus) {
  c_add(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xC7 RST 00H */
uint8_t op_C7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0000);
}
/* 0xC8 RET Z */
uint8_t op_C8(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xC9 RET */
uint8_t op_C9(struct Cpu* cpu, struct Bus* bus) {
  c_ret(cpu, bus, 1);
  return 16;
}
/* 0xCA JP Z,a16 */
uint8_t op_CA(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xCB is handled by cpu_step, should never be called */
uint8_t op_CB(struct Cpu* cpu, struct Bus* bus) {
  (void)cpu;
  (void)bus;
  return 0; /* should never be reached */
}
/* 0xCC CALL Z,a16 */
uint8_t op_CC(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, get_flag(cpu, FLG_Z));
}
/* 0xCD CALL a16 */
uint8_t op_CD(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1);
}
/* 0xCE ADC A,d8 */
uint8_t op_CE(struct Cpu* cpu, struct Bus* bus) {
  c_adc(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xCF RST 08H */
uint8_t op_CF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0008);
}
/* 0xD0 RET NC */
uint8_t op_D0(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD1 POP DE */
uint8_t op_D1(struct Cpu* cpu, struct Bus* bus) {
  st_DE(cpu, pop(cpu, bus));
  return 12;
}
/* 0xD2 JP NC,a16 */
uint8_t op_D2(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD3 illegal */
/* 0xD4 CALL NC,a16 */
uint8_t op_D4(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, 1 - get_flag(cpu, FLG_C));
}
/* 0xD5 PUSH DE */
uint8_t op_D5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_DE(cpu));
  return 16;
}
/* 0xD6 SUB d8 */
uint8_t op_D6(struct Cpu* cpu, struct Bus* bus) {
  c_sub(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xD7 RST 10H */
uint8_t op_D7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0010);
}
/* 0xD8 RET C */
uint8_t op_D8(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_ret(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xD9 RETI */
uint8_t op_D9(struct Cpu* cpu, struct Bus* bus) {
  cpu->ime = true;
  c_ret(cpu, bus, 1);
  return 16;
}
/* 0xDA JP C,a16 */
uint8_t op_DA(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_jp16(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xDB illegal */
/* 0xDC CALL C,a16 */
uint8_t op_DC(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_call(cpu, bus, get_flag(cpu, FLG_C));
}
/* 0xDD illegal */
/* 0xDE SBC A,d8 */
uint8_t op_DE(struct Cpu* cpu, struct Bus* bus) {
  c_sbc(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xDF RST 18H */
uint8_t op_DF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0018);
}
/* 0xE0 LDH (a8),A */
uint8_t op_E0(struct Cpu* cpu, struct Bus* bus) {
  uint8_t n = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  bus_write(bus, 0xFF00 + (uint16_t)n, cpu->A);
  return 12;
}
/* 0xE1 POP HL */
uint8_t op_E1(struct Cpu* cpu, struct Bus* bus) {
  st_HL(cpu, pop(cpu, bus));
  return 12;
}
/* 0xE2 LD (C),A */
uint8_t op_E2(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  bus_write(bus, 0xFF00 + cpu->C, cpu->A);
  return 8;
}
/* 0xE3 illegal */
/* 0xE4 illegal */
/* 0xE5 PUSH HL */
uint8_t op_E5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_HL(cpu));
  return 16;
}
/* 0xE6 AND d8 */
uint8_t op_E6(struct Cpu* cpu, struct Bus* bus) {
  c_and(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xE7 RST 20H */
uint8_t op_E7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0020);
}
/* 0xE8 ADD SP,r8 */
uint8_t op_E8(struct Cpu* cpu, struct Bus* bus) {
  uint8_t add = fetch_byte(cpu, bus);
  set_half_carry_add(cpu, (uint8_t)cpu->SP, add);
  set_carry_add(cpu, (uint8_t)cpu->SP, add);
  cpu->SP = (uint16_t)(cpu->SP + (int8_t)add);
  clear_flag(cpu, FLG_Z);
  clear_flag(cpu, FLG_N);
  return 16;
}
/* 0xE9 JP (HL) */
uint8_t op_E9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->PC = gt_HL(cpu);
  return 4;
}
/* 0xEA LD (a16),A */
uint8_t op_EA(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  cpu_mem_tick(cpu, bus, 12);
  bus_write(bus, addr, cpu->A);
  return 16;
}
/* 0xEB illegal */
/* 0xEC illegal */
/* 0xED illegal */
/* 0xEE XOR d8 */
uint8_t op_EE(struct Cpu* cpu, struct Bus* bus) {
  c_xor(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xEF RST 28H */
uint8_t op_EF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0028);
}
/* 0xF0 LDH A,(a8) */
uint8_t op_F0(struct Cpu* cpu, struct Bus* bus) {
  uint8_t n = fetch_byte(cpu, bus);
  cpu_mem_tick(cpu, bus, 8);
  cpu->A = bus_read(bus, 0xFF00 + (uint16_t)n);
  return 12;
}
/* 0xF1 POP AF */
uint8_t op_F1(struct Cpu* cpu, struct Bus* bus) {
  st_AF(cpu, pop(cpu, bus) & 0xFFF0);
  return 12;
}
/* 0xF2 LD A,(C) */
uint8_t op_F2(struct Cpu* cpu, struct Bus* bus) {
  cpu_mem_tick(cpu, bus, 4);
  cpu->A = bus_read(bus, 0xFF00 + cpu->C);
  return 8;
}
/* 0xF3 DI */
uint8_t op_F3(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->ime = false;
  return 4;
}
/* 0xF4 illegal */
/* 0xF5 PUSH AF */
uint8_t op_F5(struct Cpu* cpu, struct Bus* bus) {
  push(cpu, bus, gt_AF(cpu));
  return 16;
}
/* 0xF6 OR d8 */
uint8_t op_F6(struct Cpu* cpu, struct Bus* bus) {
  c_or(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xF7 RST 30H */
uint8_t op_F7(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0030);
}
/* 0xF8 LD HL,SP+r8 */
uint8_t op_F8(struct Cpu* cpu, struct Bus* bus) {
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
uint8_t op_F9(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->SP = gt_HL(cpu);
  return 8;
}
/* 0xFA LD A,(a16) */
uint8_t op_FA(struct Cpu* cpu, struct Bus* bus) {
  uint16_t addr = fetch_word(cpu, bus);
  cpu_mem_tick(cpu, bus, 12);
  cpu->A = bus_read(bus, addr);
  return 16;
}
/* 0xFB EI */
uint8_t op_FB(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  cpu->ime_pending = true;
  return 4;
}
/* 0xFC illegal */
/* 0xFD illegal */
/* 0xFE CP d8 */
uint8_t op_FE(struct Cpu* cpu, struct Bus* bus) {
  c_cp(cpu, fetch_byte(cpu, bus));
  return 8;
}
/* 0xFF RST 38H */
uint8_t op_FF(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rst(cpu, bus, 0x0038);
}
