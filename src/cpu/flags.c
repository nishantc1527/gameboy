#include "gbemu/cpu.h"
#include "internal.h"

void st_flg(uint8_t flg) { set_bit(&F, flg); }

void cl_flg(uint8_t flg) { clear_bit(&F, flg); }

void st_z(uint8_t var) {
  if (var == 0)
    st_flg(FLG_Z);
  else
    cl_flg(FLG_Z);
}

void st_h_add(uint8_t var1, uint8_t var2) {
  if (((var1 & 0xF) + (var2 & 0xF)) & 0x10)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}

void st_h_add16(uint16_t var1, uint16_t var2) {
  if (((var1 & 0xFFF) + (var2 & 0xFFF)) & 0x1000)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}

void st_h_sub(uint8_t var1, uint8_t var2) {
  if (((var1 & 0x0F) - (var2 & 0x0F)) & 0x10)
    st_flg(FLG_H);
  else
    cl_flg(FLG_H);
}

void st_c_rl(uint8_t var) {
  if (var >> 7)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}

void st_c_rr(uint8_t var) {
  if (var & 1)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}

void st_c_add(uint8_t var1, uint8_t var2) {
  uint16_t res = (uint16_t)var1 + (uint16_t)var2;
  if (res > 0xFF)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}

void st_c_add16(uint16_t var1, uint16_t var2) {
  int res = (int)var1 + (int)var2;
  if (res > 0xFFFF)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}

void st_c_sub(uint8_t var1, uint8_t var2) {
  if (var1 < var2)
    st_flg(FLG_C);
  else
    cl_flg(FLG_C);
}
