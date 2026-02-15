#pragma once

#include <stdint.h>

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

// Executes instruction, returns clock cycles taken
int c_adc(uint8_t reg);
int c_add(uint8_t reg);
int c_and(uint8_t reg);
int c_bit(uint8_t reg, uint8_t bit);
int c_call(int flg);
int c_cp(uint8_t reg);
int c_cpl(uint8_t* reg);
int c_dec(uint8_t* reg);
int c_dec_mem(uint16_t loc);
int c_inc(uint8_t* reg);
int c_inc_mem(uint16_t loc);
int c_jp8(int flg);
int c_jp16(int flg);
int c_or(uint8_t reg);
int c_res(uint8_t* reg, uint8_t bit);
int c_res_mem(uint16_t loc, int bit);
int c_ret(int flg);
int c_rr(uint8_t* reg);
int c_rr_mem(uint16_t loc);
int c_rrc(uint8_t* reg);
int c_rrc_mem(uint16_t loc);
int c_rl(uint8_t* reg);
int c_rl_mem(uint16_t loc);
int c_rlc(uint8_t* reg);
int c_rlc_mem(uint16_t loc);
int c_rst(uint8_t loc);
int c_sbc(uint8_t reg);
int c_srl(uint8_t* reg);
int c_srl_mem(uint16_t loc);
int c_set(uint8_t* reg, uint8_t bit);
int c_set_mem(uint16_t loc, uint8_t bit);
int c_sla(uint8_t* reg);
int c_sla_mem(uint16_t loc);
int c_sra(uint8_t* reg);
int c_sra_mem(uint16_t loc);
int c_sub(uint8_t reg);
int c_swp(uint8_t* reg);
int c_swp_mem(uint16_t loc);
int c_xor(uint8_t reg);

// Print instruction to stdout (disassembly)
int print_instr(uint8_t instr, uint8_t prfx);

// Utility functions for setting flags
uint8_t gt_flg(uint8_t flg);
void st_flg(uint8_t flg);
void cl_flg(uint8_t flg);
void st_z(uint8_t var);
void st_h_add(uint8_t var1, uint8_t var2);
void st_h_add16(uint16_t var1, uint16_t var2);
void st_h_sub(uint8_t var1, uint8_t var2);
void st_c_rl(uint8_t var);
void st_c_rr(uint8_t var);
void st_c_add(uint8_t var1, uint8_t var2);
void st_c_add16(uint16_t var1, uint16_t var2);
void st_c_sub(uint8_t var1, uint8_t var2);

// Interrupt functions
void req_intr(uint8_t intr);
void do_intr(uint8_t intr);

// Get and set 16-bit registers
uint16_t gt_AF(void);
void st_AF(uint16_t AF);
uint16_t gt_BC(void);
void st_BC(uint16_t BC);
uint16_t gt_DE(void);
void st_DE(uint16_t DE);
uint16_t gt_HL(void);
void st_HL(uint16_t HL);
