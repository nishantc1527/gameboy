#include <stdint.h>
#include <stdio.h>

#include "gbemu/cpu.h"
#include "gbemu/gbemu.h"
#include "gbemu/mmu.h"
#include "internal.h"

int step(struct Cpu* cpu, Mmu* mmu, struct Apu* apu, uint8_t disassemble_enable,
         int test_category, uint8_t* b_done, const uint16_t* watch_addrs,
         uint8_t watch_count) {
  if (cpu->bHALT) return 4;
  uint8_t instr = rd8(cpu, mmu);
  if (instr == 0xCB) {
    uint8_t prfx = rd8(cpu, mmu);
    if (disassemble_enable && disassemble(cpu, mmu, instr, prfx, watch_addrs, watch_count)) {
    }  // return -1 when completing disassembler
    switch (prfx) {
      case 0x00: return c_rlc(cpu, &cpu->B);
      case 0x01: return c_rlc(cpu, &cpu->C);
      case 0x02: return c_rlc(cpu, &cpu->D);
      case 0x03: return c_rlc(cpu, &cpu->E);
      case 0x04: return c_rlc(cpu, &cpu->H);
      case 0x05: return c_rlc(cpu, &cpu->L);
      case 0x06: return c_rlc_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x07: return c_rlc(cpu, &cpu->A);
      case 0x08: return c_rrc(cpu, &cpu->B);
      case 0x09: return c_rrc(cpu, &cpu->C);
      case 0x0A: return c_rrc(cpu, &cpu->D);
      case 0x0B: return c_rrc(cpu, &cpu->E);
      case 0x0C: return c_rrc(cpu, &cpu->H);
      case 0x0D: return c_rrc(cpu, &cpu->L);
      case 0x0E: return c_rrc_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x0F: return c_rrc(cpu, &cpu->A);
      case 0x10: return c_rl(cpu, &cpu->B);
      case 0x11: return c_rl(cpu, &cpu->C);
      case 0x12: return c_rl(cpu, &cpu->D);
      case 0x13: return c_rl(cpu, &cpu->E);
      case 0x14: return c_rl(cpu, &cpu->H);
      case 0x15: return c_rl(cpu, &cpu->L);
      case 0x16: return c_rl_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x17: return c_rl(cpu, &cpu->A);
      case 0x18: return c_rr(cpu, &cpu->B);
      case 0x19: return c_rr(cpu, &cpu->C);
      case 0x1A: return c_rr(cpu, &cpu->D);
      case 0x1B: return c_rr(cpu, &cpu->E);
      case 0x1C: return c_rr(cpu, &cpu->H);
      case 0x1D: return c_rr(cpu, &cpu->L);
      case 0x1E: return c_rr_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x1F: return c_rr(cpu, &cpu->A);
      case 0x20: return c_sla(cpu, &cpu->B);
      case 0x21: return c_sla(cpu, &cpu->C);
      case 0x22: return c_sla(cpu, &cpu->D);
      case 0x23: return c_sla(cpu, &cpu->E);
      case 0x24: return c_sla(cpu, &cpu->H);
      case 0x25: return c_sla(cpu, &cpu->L);
      case 0x26: return c_sla_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x27: return c_sla(cpu, &cpu->A);
      case 0x28: return c_sra(cpu, &cpu->B);
      case 0x29: return c_sra(cpu, &cpu->C);
      case 0x2A: return c_sra(cpu, &cpu->D);
      case 0x2B: return c_sra(cpu, &cpu->E);
      case 0x2C: return c_sra(cpu, &cpu->H);
      case 0x2D: return c_sra(cpu, &cpu->L);
      case 0x2E: return c_sra_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x2F: return c_sra(cpu, &cpu->A);
      case 0x30: return c_swp(cpu, &cpu->B);
      case 0x31: return c_swp(cpu, &cpu->C);
      case 0x32: return c_swp(cpu, &cpu->D);
      case 0x33: return c_swp(cpu, &cpu->E);
      case 0x34: return c_swp(cpu, &cpu->H);
      case 0x35: return c_swp(cpu, &cpu->L);
      case 0x36: return c_swp_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x37: return c_swp(cpu, &cpu->A);
      case 0x38: return c_srl(cpu, &cpu->B);
      case 0x39: return c_srl(cpu, &cpu->C);
      case 0x3A: return c_srl(cpu, &cpu->D);
      case 0x3B: return c_srl(cpu, &cpu->E);
      case 0x3C: return c_srl(cpu, &cpu->H);
      case 0x3D: return c_srl(cpu, &cpu->L);
      case 0x3E: return c_srl_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x3F: return c_srl(cpu, &cpu->A);
      case 0x40: return c_bit(cpu, cpu->B, 0);
      case 0x41: return c_bit(cpu, cpu->C, 0);
      case 0x42: return c_bit(cpu, cpu->D, 0);
      case 0x43: return c_bit(cpu, cpu->E, 0);
      case 0x44: return c_bit(cpu, cpu->H, 0);
      case 0x45: return c_bit(cpu, cpu->L, 0);
      case 0x46:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 0);
        return 12;
      case 0x47: return c_bit(cpu, cpu->A, 0);
      case 0x48: return c_bit(cpu, cpu->B, 1);
      case 0x49: return c_bit(cpu, cpu->C, 1);
      case 0x4A: return c_bit(cpu, cpu->D, 1);
      case 0x4B: return c_bit(cpu, cpu->E, 1);
      case 0x4C: return c_bit(cpu, cpu->H, 1);
      case 0x4D: return c_bit(cpu, cpu->L, 1);
      case 0x4E:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 1);
        return 12;
      case 0x4F: return c_bit(cpu, cpu->A, 1);
      case 0x50: return c_bit(cpu, cpu->B, 2);
      case 0x51: return c_bit(cpu, cpu->C, 2);
      case 0x52: return c_bit(cpu, cpu->D, 2);
      case 0x53: return c_bit(cpu, cpu->E, 2);
      case 0x54: return c_bit(cpu, cpu->H, 2);
      case 0x55: return c_bit(cpu, cpu->L, 2);
      case 0x56:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 2);
        return 12;
      case 0x57: return c_bit(cpu, cpu->A, 2);
      case 0x58: return c_bit(cpu, cpu->B, 3);
      case 0x59: return c_bit(cpu, cpu->C, 3);
      case 0x5A: return c_bit(cpu, cpu->D, 3);
      case 0x5B: return c_bit(cpu, cpu->E, 3);
      case 0x5C: return c_bit(cpu, cpu->H, 3);
      case 0x5D: return c_bit(cpu, cpu->L, 3);
      case 0x5E:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 3);
        return 12;
      case 0x5F: return c_bit(cpu, cpu->A, 3);
      case 0x60: return c_bit(cpu, cpu->B, 4);
      case 0x61: return c_bit(cpu, cpu->C, 4);
      case 0x62: return c_bit(cpu, cpu->D, 4);
      case 0x63: return c_bit(cpu, cpu->E, 4);
      case 0x64: return c_bit(cpu, cpu->H, 4);
      case 0x65: return c_bit(cpu, cpu->L, 4);
      case 0x66:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 4);
        return 12;
      case 0x67: return c_bit(cpu, cpu->A, 4);
      case 0x68: return c_bit(cpu, cpu->B, 5);
      case 0x69: return c_bit(cpu, cpu->C, 5);
      case 0x6A: return c_bit(cpu, cpu->D, 5);
      case 0x6B: return c_bit(cpu, cpu->E, 5);
      case 0x6C: return c_bit(cpu, cpu->H, 5);
      case 0x6D: return c_bit(cpu, cpu->L, 5);
      case 0x6E:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 5);
        return 12;
      case 0x6F: return c_bit(cpu, cpu->A, 5);
      case 0x70: return c_bit(cpu, cpu->B, 6);
      case 0x71: return c_bit(cpu, cpu->C, 6);
      case 0x72: return c_bit(cpu, cpu->D, 6);
      case 0x73: return c_bit(cpu, cpu->E, 6);
      case 0x74: return c_bit(cpu, cpu->H, 6);
      case 0x75: return c_bit(cpu, cpu->L, 6);
      case 0x76:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 6);
        return 12;
      case 0x77: return c_bit(cpu, cpu->A, 6);
      case 0x78: return c_bit(cpu, cpu->B, 7);
      case 0x79: return c_bit(cpu, cpu->C, 7);
      case 0x7A: return c_bit(cpu, cpu->D, 7);
      case 0x7B: return c_bit(cpu, cpu->E, 7);
      case 0x7C: return c_bit(cpu, cpu->H, 7);
      case 0x7D: return c_bit(cpu, cpu->L, 7);
      case 0x7E:
        mem_tick(cpu, mmu, apu, 8);
        c_bit(cpu, mmu_r_mem(mmu, gt_HL(cpu)), 7);
        return 12;
      case 0x7F: return c_bit(cpu, cpu->A, 7);
      case 0x80: return c_res(&cpu->B, 0);
      case 0x81: return c_res(&cpu->C, 0);
      case 0x82: return c_res(&cpu->D, 0);
      case 0x83: return c_res(&cpu->E, 0);
      case 0x84: return c_res(&cpu->H, 0);
      case 0x85: return c_res(&cpu->L, 0);
      case 0x86: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 0);
      case 0x87: return c_res(&cpu->A, 0);
      case 0x88: return c_res(&cpu->B, 1);
      case 0x89: return c_res(&cpu->C, 1);
      case 0x8A: return c_res(&cpu->D, 1);
      case 0x8B: return c_res(&cpu->E, 1);
      case 0x8C: return c_res(&cpu->H, 1);
      case 0x8D: return c_res(&cpu->L, 1);
      case 0x8E: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 1);
      case 0x8F: return c_res(&cpu->A, 1);
      case 0x90: return c_res(&cpu->B, 2);
      case 0x91: return c_res(&cpu->C, 2);
      case 0x92: return c_res(&cpu->D, 2);
      case 0x93: return c_res(&cpu->E, 2);
      case 0x94: return c_res(&cpu->H, 2);
      case 0x95: return c_res(&cpu->L, 2);
      case 0x96: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 2);
      case 0x97: return c_res(&cpu->A, 2);
      case 0x98: return c_res(&cpu->B, 3);
      case 0x99: return c_res(&cpu->C, 3);
      case 0x9A: return c_res(&cpu->D, 3);
      case 0x9B: return c_res(&cpu->E, 3);
      case 0x9C: return c_res(&cpu->H, 3);
      case 0x9D: return c_res(&cpu->L, 3);
      case 0x9E: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 3);
      case 0x9F: return c_res(&cpu->A, 3);
      case 0xA0: return c_res(&cpu->B, 4);
      case 0xA1: return c_res(&cpu->C, 4);
      case 0xA2: return c_res(&cpu->D, 4);
      case 0xA3: return c_res(&cpu->E, 4);
      case 0xA4: return c_res(&cpu->H, 4);
      case 0xA5: return c_res(&cpu->L, 4);
      case 0xA6: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 4);
      case 0xA7: return c_res(&cpu->A, 4);
      case 0xA8: return c_res(&cpu->B, 5);
      case 0xA9: return c_res(&cpu->C, 5);
      case 0xAA: return c_res(&cpu->D, 5);
      case 0xAB: return c_res(&cpu->E, 5);
      case 0xAC: return c_res(&cpu->H, 5);
      case 0xAD: return c_res(&cpu->L, 5);
      case 0xAE: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 5);
      case 0xAF: return c_res(&cpu->A, 5);
      case 0xB0: return c_res(&cpu->B, 6);
      case 0xB1: return c_res(&cpu->C, 6);
      case 0xB2: return c_res(&cpu->D, 6);
      case 0xB3: return c_res(&cpu->E, 6);
      case 0xB4: return c_res(&cpu->H, 6);
      case 0xB5: return c_res(&cpu->L, 6);
      case 0xB6: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 6);
      case 0xB7: return c_res(&cpu->A, 6);
      case 0xB8: return c_res(&cpu->B, 7);
      case 0xB9: return c_res(&cpu->C, 7);
      case 0xBA: return c_res(&cpu->D, 7);
      case 0xBB: return c_res(&cpu->E, 7);
      case 0xBC: return c_res(&cpu->H, 7);
      case 0xBD: return c_res(&cpu->L, 7);
      case 0xBE: return c_res_mem(cpu, mmu, apu, gt_HL(cpu), 7);
      case 0xBF: return c_res(&cpu->A, 7);
      case 0xC0: return c_set(&cpu->B, 0);
      case 0xC1: return c_set(&cpu->C, 0);
      case 0xC2: return c_set(&cpu->D, 0);
      case 0xC3: return c_set(&cpu->E, 0);
      case 0xC4: return c_set(&cpu->H, 0);
      case 0xC5: return c_set(&cpu->L, 0);
      case 0xC6: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 0);
      case 0xC7: return c_set(&cpu->A, 0);
      case 0xC8: return c_set(&cpu->B, 1);
      case 0xC9: return c_set(&cpu->C, 1);
      case 0xCA: return c_set(&cpu->D, 1);
      case 0xCB: return c_set(&cpu->E, 1);
      case 0xCC: return c_set(&cpu->H, 1);
      case 0xCD: return c_set(&cpu->L, 1);
      case 0xCE: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 1);
      case 0xCF: return c_set(&cpu->A, 1);
      case 0xD0: return c_set(&cpu->B, 2);
      case 0xD1: return c_set(&cpu->C, 2);
      case 0xD2: return c_set(&cpu->D, 2);
      case 0xD3: return c_set(&cpu->E, 2);
      case 0xD4: return c_set(&cpu->H, 2);
      case 0xD5: return c_set(&cpu->L, 2);
      case 0xD6: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 2);
      case 0xD7: return c_set(&cpu->A, 2);
      case 0xD8: return c_set(&cpu->B, 3);
      case 0xD9: return c_set(&cpu->C, 3);
      case 0xDA: return c_set(&cpu->D, 3);
      case 0xDB: return c_set(&cpu->E, 3);
      case 0xDC: return c_set(&cpu->H, 3);
      case 0xDD: return c_set(&cpu->L, 3);
      case 0xDE: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 3);
      case 0xDF: return c_set(&cpu->A, 3);
      case 0xE0: return c_set(&cpu->B, 4);
      case 0xE1: return c_set(&cpu->C, 4);
      case 0xE2: return c_set(&cpu->D, 4);
      case 0xE3: return c_set(&cpu->E, 4);
      case 0xE4: return c_set(&cpu->H, 4);
      case 0xE5: return c_set(&cpu->L, 4);
      case 0xE6: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 4);
      case 0xE7: return c_set(&cpu->A, 4);
      case 0xE8: return c_set(&cpu->B, 5);
      case 0xE9: return c_set(&cpu->C, 5);
      case 0xEA: return c_set(&cpu->D, 5);
      case 0xEB: return c_set(&cpu->E, 5);
      case 0xEC: return c_set(&cpu->H, 5);
      case 0xED: return c_set(&cpu->L, 5);
      case 0xEE: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 5);
      case 0xEF: return c_set(&cpu->A, 5);
      case 0xF0: return c_set(&cpu->B, 6);
      case 0xF1: return c_set(&cpu->C, 6);
      case 0xF2: return c_set(&cpu->D, 6);
      case 0xF3: return c_set(&cpu->E, 6);
      case 0xF4: return c_set(&cpu->H, 6);
      case 0xF5: return c_set(&cpu->L, 6);
      case 0xF6: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 6);
      case 0xF7: return c_set(&cpu->A, 6);
      case 0xF8: return c_set(&cpu->B, 7);
      case 0xF9: return c_set(&cpu->C, 7);
      case 0xFA: return c_set(&cpu->D, 7);
      case 0xFB: return c_set(&cpu->E, 7);
      case 0xFC: return c_set(&cpu->H, 7);
      case 0xFD: return c_set(&cpu->L, 7);
      case 0xFE: return c_set_mem(cpu, mmu, apu, gt_HL(cpu), 7);
      case 0xFF: return c_set(&cpu->A, 7);
      default:
        fprintf(stderr, "UNIMPLEMENTED PREFIX INSTRUCTION\n");
        disassemble(cpu, mmu, instr, prfx, watch_addrs, watch_count);
        return -1;
    }
  } else {
    if (disassemble_enable && disassemble(cpu, mmu, instr, 0, watch_addrs, watch_count)) {
    }  // return -1 when completing disassembler
    switch (instr) {
      case 0x00: return 4;
      case 0x01: st_BC(cpu, rd16(cpu, mmu)); return 12;
      case 0x02:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_BC(cpu), cpu->A);
        return 8;
      case 0x03: st_BC(cpu, gt_BC(cpu) + 1); return 8;
      case 0x04: return c_inc(cpu, &cpu->B);
      case 0x05: return c_dec(cpu, &cpu->B);
      case 0x06: cpu->B = rd8(cpu, mmu); return 8;
      case 0x07:
        c_rlc(cpu, &cpu->A);
        clear_flag(cpu, FLG_Z);
        return 4;
      case 0x08: {
        uint16_t addr = rd16(cpu, mmu);
        mmu_w_mem(mmu, addr, (uint8_t)(cpu->SP & 0xFF));
        mmu_w_mem(mmu, addr + 1, (uint8_t)((cpu->SP >> 8) & 0xFF));
        return 20;
      }
      case 0x09:
        st_h_add16(cpu, gt_HL(cpu), gt_BC(cpu));
        st_c_add16(cpu, gt_HL(cpu), gt_BC(cpu));
        st_HL(cpu, gt_HL(cpu) + gt_BC(cpu));
        clear_flag(cpu, FLG_N);
        return 8;
      case 0x0A:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, gt_BC(cpu));
        return 8;
      case 0x0B: st_BC(cpu, gt_BC(cpu) - 1); return 8;
      case 0x0C: return c_inc(cpu, &cpu->C);
      case 0x0D: return c_dec(cpu, &cpu->C);
      case 0x0E: cpu->C = rd8(cpu, mmu); return 8;
      case 0x0F:
        c_rrc(cpu, &cpu->A);
        clear_flag(cpu, FLG_Z);
        return 4;
      case 0x10: return 4;
      case 0x11: st_DE(cpu, rd16(cpu, mmu)); return 12;
      case 0x12:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_DE(cpu), cpu->A);
        return 8;
      case 0x13: st_DE(cpu, gt_DE(cpu) + 1); return 8;
      case 0x14: return c_inc(cpu, &cpu->D);
      case 0x15: return c_dec(cpu, &cpu->D);
      case 0x16: cpu->D = rd8(cpu, mmu); return 8;
      case 0x17:
        c_rl(cpu, &cpu->A);
        clear_flag(cpu, FLG_Z);
        return 4;
      case 0x18: {
        int8_t offset = (int8_t)rd8(cpu, mmu);
        cpu->PC = (uint16_t)(cpu->PC + offset);
        return 12;
      }
      case 0x19:
        st_h_add16(cpu, gt_HL(cpu), gt_DE(cpu));
        st_c_add16(cpu, gt_HL(cpu), gt_DE(cpu));
        st_HL(cpu, gt_HL(cpu) + gt_DE(cpu));
        clear_flag(cpu, FLG_N);
        return 8;
      case 0x1A:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, gt_DE(cpu));
        return 8;
      case 0x1B: st_DE(cpu, gt_DE(cpu) - 1); return 8;
      case 0x1C: return c_inc(cpu, &cpu->E);
      case 0x1D: return c_dec(cpu, &cpu->E);
      case 0x1E: cpu->E = rd8(cpu, mmu); return 8;
      case 0x1F:
        c_rr(cpu, &cpu->A);
        clear_flag(cpu, FLG_Z);
        return 4;
      case 0x20: return c_jp8(cpu, mmu, 1 - get_flag(cpu, FLG_Z));
      case 0x21: st_HL(cpu, rd16(cpu, mmu)); return 12;
      case 0x22:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->A);
        st_HL(cpu, gt_HL(cpu) + 1);
        return 8;
      case 0x23: st_HL(cpu, gt_HL(cpu) + 1); return 8;
      case 0x24: return c_inc(cpu, &cpu->H);
      case 0x25: return c_dec(cpu, &cpu->H);
      case 0x26: cpu->H = rd8(cpu, mmu); return 8;
      case 0x27:  // Taken from here:
                  // https://forums.nesdev.org/viewtopic.php?p=196282#p196282
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
        st_z(cpu, cpu->A);
        clear_flag(cpu, FLG_H);
        return 4;
      case 0x28: return c_jp8(cpu, mmu, get_flag(cpu, FLG_Z));
      case 0x29:
        st_h_add16(cpu, gt_HL(cpu), gt_HL(cpu));
        st_c_add16(cpu, gt_HL(cpu), gt_HL(cpu));
        st_HL(cpu, gt_HL(cpu) + gt_HL(cpu));
        clear_flag(cpu, FLG_N);
        return 8;
      case 0x2A:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, gt_HL(cpu));
        st_HL(cpu, gt_HL(cpu) + 1);
        return 8;
      case 0x2B: st_HL(cpu, gt_HL(cpu) - 1); return 8;
      case 0x2C: return c_inc(cpu, &cpu->L);
      case 0x2D: return c_dec(cpu, &cpu->L);
      case 0x2E: cpu->L = rd8(cpu, mmu); return 8;
      case 0x2F: return c_cpl(cpu, &cpu->A);
      case 0x30: return c_jp8(cpu, mmu, 1 - get_flag(cpu, FLG_C));
      case 0x31: cpu->SP = rd16(cpu, mmu); return 12;
      case 0x32:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->A);
        st_HL(cpu, gt_HL(cpu) - 1);
        return 8;
      case 0x33: ++cpu->SP; return 8;
      case 0x34: return c_inc_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x35: return c_dec_mem(cpu, mmu, apu, gt_HL(cpu));
      case 0x36: {
        uint8_t imm = rd8(cpu, mmu);
        mem_tick(cpu, mmu, apu, 8);
        mmu_w_mem(mmu, gt_HL(cpu), imm);
        return 12;
      }
      case 0x37:
        clear_flag(cpu, FLG_N);
        clear_flag(cpu, FLG_H);
        set_flag(cpu, FLG_C);
        return 4;
      case 0x38: return c_jp8(cpu, mmu, get_flag(cpu, FLG_C));
      case 0x39:
        st_h_add16(cpu, gt_HL(cpu), cpu->SP);
        st_c_add16(cpu, gt_HL(cpu), cpu->SP);
        st_HL(cpu, gt_HL(cpu) + cpu->SP);
        clear_flag(cpu, FLG_N);
        return 8;
      case 0x3A:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, gt_HL(cpu));
        st_HL(cpu, gt_HL(cpu) - 1);
        return 8;
      case 0x3B: --cpu->SP; return 8;
      case 0x3C: return c_inc(cpu, &cpu->A);
      case 0x3D: return c_dec(cpu, &cpu->A);
      case 0x3E: cpu->A = rd8(cpu, mmu); return 8;
      case 0x3F:
        clear_flag(cpu, FLG_N);
        clear_flag(cpu, FLG_H);
        if (get_flag(cpu, FLG_C)) clear_flag(cpu, FLG_C);
        else
          set_flag(cpu, FLG_C);
        return 4;
      case 0x40:
        cpu->B = cpu->B;  // NOLINT
        if (test_category == TEST_MOONEYE) {
          if (cpu->B == 3 && cpu->C == 5 && cpu->D == 8 && cpu->E == 13 &&
              cpu->H == 21 && cpu->L == 34)
            printf("PASSED\n");
          else
            printf("FAILED\n");
          *b_done = 1;
        } else if (test_category == TEST_ACID2) {
          *b_done = 1;
        }
        return 4;
      case 0x41: cpu->B = cpu->C; return 4;
      case 0x42: cpu->B = cpu->D; return 4;
      case 0x43: cpu->B = cpu->E; return 4;
      case 0x44: cpu->B = cpu->H; return 4;
      case 0x45: cpu->B = cpu->L; return 4;
      case 0x46:
        mem_tick(cpu, mmu, apu, 4);
        cpu->B = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x47: cpu->B = cpu->A; return 4;
      case 0x48: cpu->C = cpu->B; return 4;
      case 0x49:
        cpu->C = cpu->C;  // NOLINT
        return 4;
      case 0x4A: cpu->C = cpu->D; return 4;
      case 0x4B: cpu->C = cpu->E; return 4;
      case 0x4C: cpu->C = cpu->H; return 4;
      case 0x4D: cpu->C = cpu->L; return 4;
      case 0x4E:
        mem_tick(cpu, mmu, apu, 4);
        cpu->C = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x4F: cpu->C = cpu->A; return 4;
      case 0x50: cpu->D = cpu->B; return 4;
      case 0x51: cpu->D = cpu->C; return 4;
      case 0x52:
        cpu->D = cpu->D;  // NOLINT
        return 4;
      case 0x53: cpu->D = cpu->E; return 4;
      case 0x54: cpu->D = cpu->H; return 4;
      case 0x55: cpu->D = cpu->L; return 4;
      case 0x56:
        mem_tick(cpu, mmu, apu, 4);
        cpu->D = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x57: cpu->D = cpu->A; return 4;
      case 0x58: cpu->E = cpu->B; return 4;
      case 0x59: cpu->E = cpu->C; return 4;
      case 0x5A: cpu->E = cpu->D; return 4;
      case 0x5B:
        cpu->E = cpu->E;  // NOLINT
        return 4;
      case 0x5C: cpu->E = cpu->H; return 4;
      case 0x5D: cpu->E = cpu->L; return 4;
      case 0x5E:
        mem_tick(cpu, mmu, apu, 4);
        cpu->E = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x5F: cpu->E = cpu->A; return 4;
      case 0x60: cpu->H = cpu->B; return 4;
      case 0x61: cpu->H = cpu->C; return 4;
      case 0x62: cpu->H = cpu->D; return 4;
      case 0x63: cpu->H = cpu->E; return 4;
      case 0x64:
        cpu->H = cpu->H;  // NOLINT
        return 4;
      case 0x65: cpu->H = cpu->L; return 4;
      case 0x66:
        mem_tick(cpu, mmu, apu, 4);
        cpu->H = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x67: cpu->H = cpu->A; return 4;
      case 0x68: cpu->L = cpu->B; return 4;
      case 0x69: cpu->L = cpu->C; return 4;
      case 0x6A: cpu->L = cpu->D; return 4;
      case 0x6B: cpu->L = cpu->E; return 4;
      case 0x6C: cpu->L = cpu->H; return 4;
      case 0x6D:
        cpu->L = cpu->L;  // NOLINT
        return 4;
      case 0x6E:
        mem_tick(cpu, mmu, apu, 4);
        cpu->L = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x6F: cpu->L = cpu->A; return 4;
      case 0x70:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->B);
        return 8;
      case 0x71:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->C);
        return 8;
      case 0x72:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->D);
        return 8;
      case 0x73:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->E);
        return 8;
      case 0x74:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->H);
        return 8;
      case 0x75:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->L);
        return 8;
      case 0x76: cpu->bHALT = 1; return 4;
      case 0x77:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, gt_HL(cpu), cpu->A);
        return 8;
      case 0x78: cpu->A = cpu->B; return 4;
      case 0x79: cpu->A = cpu->C; return 4;
      case 0x7A: cpu->A = cpu->D; return 4;
      case 0x7B: cpu->A = cpu->E; return 4;
      case 0x7C: cpu->A = cpu->H; return 4;
      case 0x7D: cpu->A = cpu->L; return 4;
      case 0x7E:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, gt_HL(cpu));
        return 8;
      case 0x7F:
        cpu->A = cpu->A;  // NOLINT
        return 4;
      case 0x80: return c_add(cpu, cpu->B);
      case 0x81: return c_add(cpu, cpu->C);
      case 0x82: return c_add(cpu, cpu->D);
      case 0x83: return c_add(cpu, cpu->E);
      case 0x84: return c_add(cpu, cpu->H);
      case 0x85: return c_add(cpu, cpu->L);
      case 0x86:
        mem_tick(cpu, mmu, apu, 4);
        c_add(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0x87: return c_add(cpu, cpu->A);
      case 0x88: return c_adc(cpu, cpu->B);
      case 0x89: return c_adc(cpu, cpu->C);
      case 0x8A: return c_adc(cpu, cpu->D);
      case 0x8B: return c_adc(cpu, cpu->E);
      case 0x8C: return c_adc(cpu, cpu->H);
      case 0x8D: return c_adc(cpu, cpu->L);
      case 0x8E:
        mem_tick(cpu, mmu, apu, 4);
        c_adc(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0x8F: return c_adc(cpu, cpu->A);
      case 0x90: return c_sub(cpu, cpu->B);
      case 0x91: return c_sub(cpu, cpu->C);
      case 0x92: return c_sub(cpu, cpu->D);
      case 0x93: return c_sub(cpu, cpu->E);
      case 0x94: return c_sub(cpu, cpu->H);
      case 0x95: return c_sub(cpu, cpu->L);
      case 0x96:
        mem_tick(cpu, mmu, apu, 4);
        c_sub(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0x97: return c_sub(cpu, cpu->A);
      case 0x98: return c_sbc(cpu, cpu->B);
      case 0x99: return c_sbc(cpu, cpu->C);
      case 0x9A: return c_sbc(cpu, cpu->D);
      case 0x9B: return c_sbc(cpu, cpu->E);
      case 0x9C: return c_sbc(cpu, cpu->H);
      case 0x9D: return c_sbc(cpu, cpu->L);
      case 0x9E:
        mem_tick(cpu, mmu, apu, 4);
        c_sbc(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0x9F: return c_sbc(cpu, cpu->A);
      case 0xA0: return c_and(cpu, cpu->B);
      case 0xA1: return c_and(cpu, cpu->C);
      case 0xA2: return c_and(cpu, cpu->D);
      case 0xA3: return c_and(cpu, cpu->E);
      case 0xA4: return c_and(cpu, cpu->H);
      case 0xA5: return c_and(cpu, cpu->L);
      case 0xA6:
        mem_tick(cpu, mmu, apu, 4);
        c_and(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0xA7: return c_and(cpu, cpu->A);
      case 0xA8: return c_xor(cpu, cpu->B);
      case 0xA9: return c_xor(cpu, cpu->C);
      case 0xAA: return c_xor(cpu, cpu->D);
      case 0xAB: return c_xor(cpu, cpu->E);
      case 0xAC: return c_xor(cpu, cpu->H);
      case 0xAD: return c_xor(cpu, cpu->L);
      case 0xAE:
        mem_tick(cpu, mmu, apu, 4);
        c_xor(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0xAF: return c_xor(cpu, cpu->A);
      case 0xB0: return c_or(cpu, cpu->B);
      case 0xB1: return c_or(cpu, cpu->C);
      case 0xB2: return c_or(cpu, cpu->D);
      case 0xB3: return c_or(cpu, cpu->E);
      case 0xB4: return c_or(cpu, cpu->H);
      case 0xB5: return c_or(cpu, cpu->L);
      case 0xB6:
        mem_tick(cpu, mmu, apu, 4);
        c_or(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0xB7: return c_or(cpu, cpu->A);
      case 0xB8: return c_cp(cpu, cpu->B);
      case 0xB9: return c_cp(cpu, cpu->C);
      case 0xBA: return c_cp(cpu, cpu->D);
      case 0xBB: return c_cp(cpu, cpu->E);
      case 0xBC: return c_cp(cpu, cpu->H);
      case 0xBD: return c_cp(cpu, cpu->L);
      case 0xBE:
        mem_tick(cpu, mmu, apu, 4);
        c_cp(cpu, mmu_r_mem(mmu, gt_HL(cpu)));
        return 8;
      case 0xBF: return c_cp(cpu, cpu->A);
      case 0xC0: return c_ret(cpu, mmu, 1 - get_flag(cpu, FLG_Z));
      case 0xC1: st_BC(cpu, pop(cpu, mmu)); return 12;
      case 0xC2: return c_jp16(cpu, mmu, 1 - get_flag(cpu, FLG_Z));
      case 0xC3: return c_jp16(cpu, mmu, 1);
      case 0xC4: return c_call(cpu, mmu, 1 - get_flag(cpu, FLG_Z));
      case 0xC5: push(cpu, mmu, gt_BC(cpu)); return 16;
      case 0xC6: c_add(cpu, rd8(cpu, mmu)); return 8;
      case 0xC7: return c_rst(cpu, mmu, 0x0000);
      case 0xC8: return c_ret(cpu, mmu, get_flag(cpu, FLG_Z));
      case 0xC9: c_ret(cpu, mmu, 1); return 16;
      case 0xCA: return c_jp16(cpu, mmu, get_flag(cpu, FLG_Z));
      case 0xCC: return c_call(cpu, mmu, get_flag(cpu, FLG_Z));
      case 0xCD: return c_call(cpu, mmu, 1);
      case 0xCE: c_adc(cpu, rd8(cpu, mmu)); return 8;
      case 0xCF: return c_rst(cpu, mmu, 0x0008);
      case 0xD0: return c_ret(cpu, mmu, 1 - get_flag(cpu, FLG_C));
      case 0xD1: st_DE(cpu, pop(cpu, mmu)); return 12;
      case 0xD2: return c_jp16(cpu, mmu, 1 - get_flag(cpu, FLG_C));
      case 0xD4: return c_call(cpu, mmu, 1 - get_flag(cpu, FLG_C));
      case 0xD5: push(cpu, mmu, gt_DE(cpu)); return 16;
      case 0xD6: c_sub(cpu, rd8(cpu, mmu)); return 8;
      case 0xD7: return c_rst(cpu, mmu, 0x0010);
      case 0xD8: return c_ret(cpu, mmu, get_flag(cpu, FLG_C));
      case 0xD9:
        cpu->bIME = 1;
        c_ret(cpu, mmu, 1);
        return 16;
      case 0xDA: return c_jp16(cpu, mmu, get_flag(cpu, FLG_C));
      case 0xDC: return c_call(cpu, mmu, get_flag(cpu, FLG_C));
      case 0xDE: c_sbc(cpu, rd8(cpu, mmu)); return 8;
      case 0xDF: return c_rst(cpu, mmu, 0x0018);
      case 0xE0: {
        uint8_t n = rd8(cpu, mmu);
        mem_tick(cpu, mmu, apu, 8);
        mmu_w_mem(mmu, 0xFF00 + (uint16_t)n, cpu->A);
        return 12;
      }
      case 0xE1: st_HL(cpu, pop(cpu, mmu)); return 12;
      case 0xE2:
        mem_tick(cpu, mmu, apu, 4);
        mmu_w_mem(mmu, 0xFF00 + cpu->C, cpu->A);
        return 8;
      case 0xE5: push(cpu, mmu, gt_HL(cpu)); return 16;
      case 0xE6: c_and(cpu, rd8(cpu, mmu)); return 8;
      case 0xE7: return c_rst(cpu, mmu, 0x0020);
      case 0xE8: {
        uint8_t add = rd8(cpu, mmu);
        st_h_add(cpu, (uint8_t)cpu->SP, add);
        st_c_add(cpu, (uint8_t)cpu->SP, add);
        cpu->SP = (uint16_t)(cpu->SP + (int8_t)add);
        clear_flag(cpu, FLG_Z);
        clear_flag(cpu, FLG_N);
        return 16;
      }
      case 0xE9: cpu->PC = gt_HL(cpu); return 4;
      case 0xEA: {
        uint16_t addr = rd16(cpu, mmu);
        mem_tick(cpu, mmu, apu, 12);
        mmu_w_mem(mmu, addr, cpu->A);
        return 16;
      }
      case 0xEE: c_xor(cpu, rd8(cpu, mmu)); return 8;
      case 0xEF: return c_rst(cpu, mmu, 0x0028);
      case 0xF0: {
        uint8_t n = rd8(cpu, mmu);
        mem_tick(cpu, mmu, apu, 8);
        cpu->A = mmu_r_mem(mmu, 0xFF00 + (uint16_t)n);
        return 12;
      }
      case 0xF1: st_AF(cpu, pop(cpu, mmu) & 0xFFF0); return 12;
      case 0xF2:
        mem_tick(cpu, mmu, apu, 4);
        cpu->A = mmu_r_mem(mmu, 0xFF00 + cpu->C);
        return 8;
      case 0xF3: cpu->bIME = 0; return 4;
      case 0xF5: push(cpu, mmu, gt_AF(cpu)); return 16;
      case 0xF6: c_or(cpu, rd8(cpu, mmu)); return 8;
      case 0xF7: return c_rst(cpu, mmu, 0x0030);
      case 0xF8: {
        uint8_t nxt = rd8(cpu, mmu);
        uint16_t add = (uint16_t)(cpu->SP + (int8_t)nxt);
        st_HL(cpu, add);
        clear_flag(cpu, FLG_Z);
        clear_flag(cpu, FLG_N);
        st_h_add(cpu, (uint8_t)cpu->SP, nxt);
        st_c_add(cpu, (uint8_t)cpu->SP, nxt);
        return 12;
      }
      case 0xF9: cpu->SP = gt_HL(cpu); return 8;
      case 0xFA: {
        uint16_t addr = rd16(cpu, mmu);
        mem_tick(cpu, mmu, apu, 12);
        cpu->A = mmu_r_mem(mmu, addr);
        return 16;
      }
      case 0xFB: cpu->bIME = 1; return 4;
      case 0xFE: c_cp(cpu, rd8(cpu, mmu)); return 8;
      case 0xFF: return c_rst(cpu, mmu, 0x0038);
      default: fprintf(stderr, "UNIMPLEMENTED INSTRUCTION\n"); return -1;
    }
  }
}
