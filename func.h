#ifndef FUNC
#define FUNC

#include <stdio.h>
#include "var.h"

int gt_clr(byte pal, int val) {
  return (pal >> (val << 1)) & 0b11;
}

byte gt_bt(byte var, byte bt) {
  return (var >> bt) & 1;
}

void st_bt(byte* var, byte bt) {
  *var |= (1 << bt);
}

void cl_bt(byte* var, byte bt) {
  *var &= ~(1 << bt);
}

byte gt_flg(byte flg) {
  return gt_bt(F, flg);
}

void st_flg(byte flg) {
  st_bt(&F, flg);
}

void cl_flg(byte flg) {
  cl_bt(&F, flg);
}

void st_z(byte var) {
  if(var == 0) st_flg(FLG_Z);
  else cl_flg(FLG_Z);
}

void st_h_add(byte var1, byte var2) {
  if(((var1 & 0x0F) + (var2 & 0x0F)) & 0x10) st_flg(FLG_H);
  else cl_flg(FLG_H);
}

void st_h_add16(dbyte var1, dbyte var2) {
  if(((var1 & 0xFF) + (var2 & 0xFF)) & 0x100) st_flg(FLG_H);
  else cl_flg(FLG_H);
}

void st_h_sub(byte var1, byte var2) {
  if(((var1 & 0x0F) - (var2 & 0x0F)) & 0x10) st_flg(FLG_H);
  else cl_flg(FLG_H);
}

void st_c_rl(byte var) {
  if(var >> 7) st_flg(FLG_C);
  else cl_flg(FLG_C);
}

void st_c_add(byte var1, byte var2) {
  dbyte res = (dbyte) var1 + (dbyte) var2;
  if(res > 0xFF) st_flg(FLG_C);
  else cl_flg(FLG_C);
}

void st_c_add16(dbyte var1, dbyte var2) {
  if((int) var1 + (int) var2 > 0xFFFF) st_flg(FLG_C);
  else cl_flg(FLG_C);
}

void st_c_sub(byte var1, byte var2) {
  if(var1 < var2) st_flg(FLG_C);
  else cl_flg(FLG_C);
}

byte r_mem(dbyte loc) {
  if(mem[0xFF50] || loc >= 0x100) return mem[loc];
  else return brom[loc];
}

void w_mem(dbyte loc, byte val) {
  if(loc == 0xFF46) dma = 1;
  if (loc == 0xFF04) val = 0;
  mem[loc] = val;
}

byte rd8() {
  if(r_mem(0xFF50)) return mem[++ PC];
  else return brom[++ PC];
}

dbyte rd16() {
  dbyte addr1 = ++ PC;
  dbyte addr2 = ++ PC;
  if(r_mem(0xFF50)) return ((dbyte) mem[addr2] << 8) | (dbyte) mem[addr1];
  else return ((dbyte) brom[addr2] << 8) | (dbyte) brom[addr1];
}

void psh16(dbyte val) {
  byte val1 = (byte) (val >> 8);
  byte val2 = (byte) val;
  w_mem(SP - 1, val1);
  w_mem(SP - 2, val2);
  SP -= 2;
}

dbyte pop16() {
  dbyte val1 = r_mem(SP);
  dbyte val2 = r_mem(SP + 1);
  SP += 2;
  return val1 | (val2 << 8);
}

dbyte gt_AF() {
  return (((dbyte) A) << 8) | (dbyte) F;
}

void st_AF(dbyte AF) {
  A = (byte) (AF >> 8);
  F = (byte) AF;
}

dbyte gt_BC() {
  return (((dbyte) B) << 8) | (dbyte) C;
}

void st_BC(dbyte BC) {
  B = (byte) (BC >> 8);
  C = (byte) BC;
}

dbyte gt_DE() {
  return (((dbyte) D) << 8) | (dbyte) E;
}

void st_DE(dbyte DE) {
  D = (byte) (DE >> 8);
  E = (byte) DE;
}

dbyte gt_HL() {
  return (((dbyte) H) << 8) | (dbyte) L;
}

void st_HL(dbyte HL) {
  H = (byte) (HL >> 8);
  L = (byte) HL;
}

void kp() {
  PC --;
}

int c_add(byte reg) {
  st_h_add(A, reg);
  st_c_add(A, reg);
  A += reg;
  st_z(A);
  cl_flg(FLG_N);
  return 4;
}

int c_and(byte reg) {
  A &= reg;
  st_z(A);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  cl_flg(FLG_C);
  return 8;
}

int c_bit(byte reg, int bit) {
  if(gt_bt(reg, bit)) cl_flg(FLG_Z);
  else st_flg(FLG_Z);
  cl_flg(FLG_N);
  st_flg(FLG_H);
  return 8;
}

int c_cpl(byte* reg) {
  *reg = ~*reg;
  st_flg(FLG_N);
  st_flg(FLG_H);
  return 4;
}

int c_dec(byte* reg) {
  st_h_sub(*reg, 1);
  *reg = *reg - 1;
  st_z(*reg);
  st_flg(FLG_N);
  return 4;
}

int c_inc(byte* reg) {
  st_h_add(*reg, 1);
  *reg = *reg + 1;
  st_z(*reg);
  cl_flg(FLG_N);
  return 4;
}

int c_or(byte reg) {
  A |= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

int c_res(byte* reg, int bit) {
  *reg = *reg & ~(1 << bit);
  return 8;
}

int c_rr(byte* reg) {
  byte carry = gt_flg(FLG_C);
  if(*reg & 1) st_flg(FLG_C);
  else cl_flg(FLG_C);
  *reg = *reg >> 1;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  *reg = *reg | (carry << 7);
  return 8;
}

int c_rl(byte* reg) {
  int carry = gt_flg(FLG_C);
  st_c_rl(C);
  *reg = (*reg << 1) | carry;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_rst(byte loc) {
  psh16(PC + 1);
  PC = loc;
  kp();
  return 16;
}

int c_srl(byte* reg) {
  if(*reg & 1) st_flg(FLG_C);
  else cl_flg(FLG_C);
  *reg = *reg >> 1;
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  return 8;
}

int c_set(byte* reg, int bit) {
  st_bt(reg, bit);
  return 16;
}

int c_swp(byte* reg) {
  *reg = (*reg >> 4) | (*reg << 4);
  st_z(*reg);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 8;
}

int c_xor(int reg) {
  A ^= reg;
  st_z(A);
  cl_flg(FLG_N);
  cl_flg(FLG_H);
  cl_flg(FLG_C);
  return 4;
}

void req_intr(int intr) {
  st_bt(&mem[0xFF0F], intr);
}

void do_intr(int intr) {
    if (!HALT || IME) {
        switch (intr) {
        case 0:
            printf("VBLANK INTERRUPT\n");
            break;
        case 1:
            printf("LCD STAT INTERRUPT\n");
            break;
        case 2:
            printf("TIMER INTERRUPT\n");
            break;
        case 3:
            printf("SERIAL INTERRUPT\n");
            break;
        case 4:
            printf("JOYPAD INTERRUPT");
            break;
        }
        cl_bt(&mem[0xFF0F], intr);
        psh16(PC);
        PC = intr_loc[intr];
    }
    HALT = 0;
    IME = 0;
}

int chck_intr() {
    if (!IME) return 0;
    for (int i = 0; i <= 4; i++) {
        if (gt_bt(IF, i) && gt_bt(IE, i)) {
            do_intr(i);
            return 1;
        }
    }
    return 0;
}

void chck_in() {
    memset(in, 0, sizeof(in));
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) return 1;
        else if (evt.type == SDL_KEYDOWN) {
            switch (evt.key.keysym.sym) {
            case SDLK_s:
                in[BTN_A] = 1;
                break;
            case SDLK_a:
                in[BTN_B] = 1;
                break;
            case SDLK_RETURN:
                in[BTN_STRT] = 1;
                break;
            case SDLK_LSHIFT: case SDLK_RSHIFT:
                in[BTN_SLCT] = 1;
                break;
            case SDLK_UP:
                in[BTN_UP] = 1;
                break;
            case SDLK_DOWN:
                in[BTN_DOWN] = 1;
                break;
            case SDLK_LEFT:
                in[BTN_LEFT] = 1;
                break;
            case SDLK_RIGHT:
                in[BTN_RIGHT] = 1;
                break;
            }
        }
    }
    byte in_reg = JOYP;
    in_reg |= 0xF;
    if (!gt_bt(in_reg, 4)) {
        if (in[BTN_RIGHT]) cl_bt(&in_reg, 0);
        if (in[BTN_LEFT]) cl_bt(&in_reg, 1);
        if (in[BTN_UP]) cl_bt(&in_reg, 2);
        if (in[BTN_DOWN]) cl_bt(&in_reg, 3);
    }
    if (!gt_bt(in_reg, 5)) {
        if (in[BTN_A]) cl_bt(&in_reg, 0);
        if (in[BTN_B]) cl_bt(&in_reg, 1);
        if (in[BTN_SLCT]) cl_bt(&in_reg, 2);
        if (in[BTN_STRT]) cl_bt(&in_reg, 3);
    }
    if (in_reg != JOYP) req_intr(4);
    w_mem(0xFF00, in_reg);
}

void chck_dma() {
  if(dma) {
    dma = 0;
    byte src = DMA * 0x100;
    for(int t = 0; t < 0xA0; t ++) {
      w_mem(0xFE00 + t, r_mem(src + t));
    }
  }
}

void upd_tim() {
    if (!gt_bt(TAC, 2)) return;
    byte div = DIV;
    if (STOP) div = 0;
    w_mem(0xFF04, div);

    byte tima = TIMA;
    if (tima == 0xFF) {
        tima = TMA;
        req_intr(2);
    }
    else tima++;
    w_mem(0xFF05, tima);
}

void dbg() {
  printf("{\n\tAF: $%04X\n\tBC: $%04X\n\tDE: $%04X\n\tHL: $%04X\n\tSP: $%04X\n\tPC: $%04X\n\tZERO FLAG: %d\n\tSUBTRACTION FLAG: %d\n\tHALF CARRY FLAG: %d\n\tCARRY FLAG: %d\n}\n", gt_AF(), gt_BC(), gt_DE(), gt_HL(), SP, PC, gt_flg(FLG_Z), gt_flg(FLG_N), gt_flg(FLG_H), gt_flg(FLG_C));
}

#endif
