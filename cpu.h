#ifndef CPU
#define CPU

#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"

int exec(byte instr) {
  p_instr(instr);
  if(instr == 0xCB) {
    byte prfx = rd8();
    switch(prfx) {
      case 0x11:
        {
          int carry = gt_flg(FLG_C);
          st_c_rl(C);
          C = (C << 1) | carry;
          st_z(C);
          cl_flg(FLG_N);
          cl_flg(FLG_H);
          return 8;
        }
      case 0x7C:
        if(gt_bt(H, 7)) cl_flg(FLG_Z);
        else st_flg(FLG_Z);
        cl_flg(FLG_N);
        st_flg(FLG_H);
        return 8;
      default:
        printf("UNIMPLEMENTED PREFIX INSTRUCTION\n");
        return -1;
    }
  } else {
    switch(instr) {
      case 0x04:
        st_h_add(B, 1);
        B ++;
        st_z(B);
        cl_flg(FLG_N);
        return 4;
      case 0x05:
        st_h_sub(B, 1);
        B --;
        st_z(B);
        st_flg(FLG_N);
        return 4;
      case 0x06:
        B = rd8();
        return 8;
      case 0x0C:
        st_h_add(C, 1);
        C ++;
        st_z(C);
        cl_flg(FLG_N);
        return 4;
      case 0x0D:
        st_h_sub(C, 1);
        C --;
        st_z(C);
        st_flg(FLG_N);
        return 4;
      case 0x0E:
        C = rd8();
        return 8;
      case 0x11:
        st_DE(rd16());
        return 12;
      case 0x13:
        st_DE(gt_DE() + 1);
        return 8;
      case 0x15:
        st_h_sub(D, 1);
        D --;
        st_z(D);
        st_flg(FLG_N);
        return 4;
      case 0x17:
        {
          int carry = gt_flg(FLG_C);
          st_c_rl(A);
          A = (A << 1) | carry;
          st_z(A);
          cl_flg(FLG_N);
          cl_flg(FLG_H);
          return 8;
        }
      case 0x18:
        PC += (signed char) rd8();
        return 12;
      case 0x1A:
        A = r_mem(gt_DE());
        return 8;
      case 0x1D:
        st_h_sub(E, 1);
        E --;
        st_z(E);
        st_flg(FLG_N);
        return 4;
      case 0x1E:
        E = rd8();
        return 8;
      case 0x20:
        if(!gt_flg(FLG_Z)) {
          PC += (signed char) rd8();
          return 12;
        } else {
          rd8();
          return 8;
        }
      case 0x21:
        st_HL(rd16());
        return 12;
      case 0x22:
        w_mem(gt_HL(), A);
        st_HL(gt_HL() + 1);
        return 8;
      case 0x23:
        st_HL(gt_HL() + 1);
        return 8;
      case 0x24:
        st_h_add(H, 1);
        H ++;
        st_z(H);
        cl_flg(FLG_N);
        return 4;
      case 0x28:
        if(gt_flg(FLG_Z)) {
          PC += (signed char) rd8();
          return 12;
        } else {
          rd8();
          return 8;
        }
      case 0x2E:
        L = rd8();
        return 8;
      case 0x31:
        SP = rd16();
        return 12;
      case 0x32:
        w_mem(gt_HL(), A);
        st_HL(gt_HL() - 1);
        return 8;
      case 0x3D:
        st_h_sub(A, 1);
        A --;
        st_z(A);
        st_flg(FLG_N);
        return 4;
      case 0x3E:
        A = rd8();
        return 8;
      case 0x4F:
        C = A;
        return 4;
      case 0x57:
        D = A;
        return 4;
      case 0x67:
        H = A;
        return 4;
      case 0x77:
        w_mem(gt_HL(), A);
        return 8;
      case 0x78:
        A = B;
        return 4;
      case 0x7B:
        A = E;
        return 4;
      case 0x7C:
        A = H;
        return 4;
      case 0x7D:
        A = L;
        return 4;
      case 0x86:
        {
          byte var = r_mem(gt_HL());
          st_h_add(A, var);
          st_c_add(A, var);
          A += var;
          st_z(A);
          cl_flg(FLG_N);
          return 8;
        }
      case 0x90:
        st_h_sub(A, B);
        st_c_sub(A, B);
        A -= B;
        st_z(A);
        st_flg(FLG_N);
        return 4;
      case 0xAF:
        A = 0;
        st_flg(FLG_Z);
        cl_flg(FLG_N);
        cl_flg(FLG_H);
        cl_flg(FLG_C);
        return 4;
      case 0xBE:
        {
          byte var = r_mem(gt_HL());
          st_z(A - var);
          st_flg(FLG_N);
          st_h_sub(A, var);
          st_c_sub(A, var);
          return 8;
        }
      case 0xC1:
        st_BC(pop16());
        return 12;
      case 0xC5:
        psh16(gt_BC());
        return 16;
      case 0xC9:
        PC = pop16() + 2;
        return 16;
      case 0xCD:
        psh16(PC);
        PC = rd16() - 1;
        return 24;
      case 0xE0:
        w_mem(0xFF00 + rd8(), A);
        return 12;
      case 0xE2:
        w_mem(0xFF00 + C, A);
        return 8;
      case 0xEA:
        w_mem(rd16(), A);
        return 16;
      case 0xF0:
        A = r_mem(0xFF00 + rd8());
        return 12;
      case 0xFE:
        {
          byte var = rd8();
          st_z(A - var);
          st_flg(FLG_N);
          st_h_sub(A, var);
          st_c_sub(A, var);
          return 8;
        }
      default:
        printf("UNIMPLEMENTED INSTRUCTION\n");
        return -1;
    }
  }
  return 0;
}

#endif
