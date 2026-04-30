#include <stdint.h>

#include "../cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"
#include "ops_private.h"

/* CB 0x00-0x07: RLC r */
uint8_t cb_00(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->B);
}
uint8_t cb_01(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->C);
}
uint8_t cb_02(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->D);
}
uint8_t cb_03(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->E);
}
uint8_t cb_04(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->H);
}
uint8_t cb_05(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->L);
}
uint8_t cb_06(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rlc_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_07(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rlc(cpu, &cpu->A);
}
/* CB 0x08-0x0F: RRC r */
uint8_t cb_08(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->B);
}
uint8_t cb_09(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->C);
}
uint8_t cb_0A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->D);
}
uint8_t cb_0B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->E);
}
uint8_t cb_0C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->H);
}
uint8_t cb_0D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->L);
}
uint8_t cb_0E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rrc_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_0F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rrc(cpu, &cpu->A);
}
/* CB 0x10-0x17: RL r */
uint8_t cb_10(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->B);
}
uint8_t cb_11(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->C);
}
uint8_t cb_12(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->D);
}
uint8_t cb_13(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->E);
}
uint8_t cb_14(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->H);
}
uint8_t cb_15(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->L);
}
uint8_t cb_16(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rl_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_17(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rl(cpu, &cpu->A);
}
/* CB 0x18-0x1F: RR r */
uint8_t cb_18(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->B);
}
uint8_t cb_19(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->C);
}
uint8_t cb_1A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->D);
}
uint8_t cb_1B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->E);
}
uint8_t cb_1C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->H);
}
uint8_t cb_1D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->L);
}
uint8_t cb_1E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_rr_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_1F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_rr(cpu, &cpu->A);
}
/* CB 0x20-0x27: SLA r */
uint8_t cb_20(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->B);
}
uint8_t cb_21(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->C);
}
uint8_t cb_22(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->D);
}
uint8_t cb_23(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->E);
}
uint8_t cb_24(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->H);
}
uint8_t cb_25(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->L);
}
uint8_t cb_26(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_sla_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_27(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sla(cpu, &cpu->A);
}
/* CB 0x28-0x2F: SRA r */
uint8_t cb_28(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->B);
}
uint8_t cb_29(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->C);
}
uint8_t cb_2A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->D);
}
uint8_t cb_2B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->E);
}
uint8_t cb_2C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->H);
}
uint8_t cb_2D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->L);
}
uint8_t cb_2E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_sra_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_2F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_sra(cpu, &cpu->A);
}
/* CB 0x30-0x37: SWAP r */
uint8_t cb_30(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->B);
}
uint8_t cb_31(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->C);
}
uint8_t cb_32(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->D);
}
uint8_t cb_33(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->E);
}
uint8_t cb_34(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->H);
}
uint8_t cb_35(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->L);
}
uint8_t cb_36(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_swp_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_37(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_swp(cpu, &cpu->A);
}
/* CB 0x38-0x3F: SRL r */
uint8_t cb_38(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->B);
}
uint8_t cb_39(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->C);
}
uint8_t cb_3A(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->D);
}
uint8_t cb_3B(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->E);
}
uint8_t cb_3C(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->H);
}
uint8_t cb_3D(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->L);
}
uint8_t cb_3E(struct Cpu* cpu, struct Bus* bus) {
  return (uint8_t)c_srl_mem(cpu, bus, gt_HL(cpu));
}
uint8_t cb_3F(struct Cpu* cpu, struct Bus* bus) {
  (void)bus;
  return (uint8_t)c_srl(cpu, &cpu->A);
}
