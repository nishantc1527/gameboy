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

byte r_mem(dbyte loc) {
  if(mem[0xFF50] || loc >= 0x100) return mem[loc];
  else return brom[loc];
}

void w_mem(dbyte loc, byte val) {
  mem[loc] = val;
}

byte rd8() {
  if(r_mem(0xFF50)) return mem[PC ++];
  else return brom[PC ++];
}

dbyte rd16() {
  dbyte addr1 = PC ++;
  dbyte addr2 = PC ++;
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

#endif
