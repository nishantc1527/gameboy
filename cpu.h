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
          int carry = 0;
          if(C >> 7) carry = 1;
          C <<= 1;
          if(gt_flg(FLG_C)) C |= 1;
          if(C == 0) st_flg(FLG_Z);
          else cl_flg(FLG_Z);
          cl_flg(FLG_N);
          cl_flg(FLG_H);
          if(carry) st_flg(FLG_C);
          else cl_flg(FLG_C);
          return 8;
        }
      case 0x7C:
        if(!gt_bt(H, 7)) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        cl_flg(FLG_N);
        st_flg(FLG_H);
        return 8;
      default:
        printf("UNIMPLEMENTED PREFIX INSTRUCTION\n");
        return -1;
    }
  } else {
    switch(instr) {
      case 0x00:
        return 4;
      case 0x04:
        B ++;
        cl_flg(FLG_Z);
        cl_flg(FLG_N);
        if(B == 0x10) st_flg(FLG_H);
        else cl_flg(FLG_H);
        return 4;
      case 0x05:
        B --;
        if(B == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
        return 4;
      case 0x06:
        B = rd8();
        return 8;
      case 0x0C:
        C ++;
        cl_flg(FLG_Z);
        cl_flg(FLG_N);
        if(C == 0x10) st_flg(FLG_H);
        else cl_flg(FLG_H);
        return 4;
      case 0x0D:
        C --;
        if(C == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
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
        D --;
        if(D == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
        return 4;
      case 0x16:
        D = rd8();
        return 8;
      case 0x17:
        {
          int carry = 0;
          if(A >> 7) carry = 1;
          A <<= 1;
          if(gt_flg(FLG_C)) A |= 1;
          if(A == 0) st_flg(FLG_Z);
          else cl_flg(FLG_Z);
          cl_flg(FLG_N);
          cl_flg(FLG_H);
          if(carry) st_flg(FLG_C);
          else cl_flg(FLG_C);
          return 4;
        }
      case 0x18:
        PC += (signed char) rd8();
        return 12;
      case 0x1A:
        A = r_mem(gt_DE());
        return 8;
      case 0x1D:
        E --;
        if(E == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
        return 4;
      case 0x1E:
        E = rd8();
        return 8;
      case 0x20:
        {
          signed char addr = rd8();
          if(!gt_flg(FLG_Z)) {
            PC += addr;
            return 12;
          }
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
        H ++;
        cl_flg(FLG_Z);
        cl_flg(FLG_N);
        if(H == 0x10) st_flg(FLG_H);
        else cl_flg(FLG_H);
        return 4;
      case 0x28:
        {
          signed char addr = rd8();
          if(gt_flg(FLG_Z)) {
            PC += addr;
            return 12;
          }
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
        A --;
        if(A == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
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
        if((0x10 & A) + (0x10 & r_mem(gt_HL())) == 0x10) st_flg(FLG_H);
        else cl_flg(FLG_H);
        if((dbyte) A + (dbyte) r_mem(gt_HL()) > 0xFF) st_flg(FLG_C);
        else cl_flg(FLG_C);
        A += r_mem(gt_HL());
        if(A == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        cl_flg(FLG_N);
        return 8;
      case 0x90:
        if(A < B) st_flg(FLG_C);
        else cl_flg(FLG_C);
        A -= B;
        if(A == 0) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
        return 4;
      case 0xAF:
        A = 0;
        st_flg(FLG_Z);
        cl_flg(FLG_N);
        cl_flg(FLG_H);
        cl_flg(FLG_C);
        return 4;
      case 0xBE:
        if(r_mem(gt_HL()) == A) st_flg(FLG_Z);
        else cl_flg(FLG_Z);
        st_flg(FLG_N);
        cl_flg(FLG_H);
        if(A < r_mem(gt_HL())) st_flg(FLG_C);
        else cl_flg(FLG_C);
        return 8;
      case 0xC1:
        st_BC(pop16());
        return 12;
      case 0xC3:
        PC = rd16();
        return 16;
      case 0xC5:
        psh16(gt_BC());
        return 16;
      case 0xC9:
        PC = pop16();
        return 16;
      case 0xCD:
        psh16(PC + 2);
        PC = rd16();
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
      case 0xEF:
        psh16(PC + 1);
        // PC = 0x0028;
        return 16;
      case 0xF0:
        A = r_mem(0xFF00 + rd8());
        return 12;
      case 0xFB:
        IME = 1;
        return 4;
      case 0xFE:
        {
          byte val = rd8();
          if(A == val) st_flg(FLG_Z);
          else cl_flg(FLG_Z);
          st_flg(FLG_N);
          cl_flg(FLG_H);
          if(A < val) st_flg(FLG_C);
          else cl_flg(FLG_C);
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
