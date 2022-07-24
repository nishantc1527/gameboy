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
        return c_rl(&C);
      case 0x19:
        return c_rr(&C);
      case 0x1A:
        return c_rr(&D);
      case 0x37:
        return c_swp(&A);
      case 0x38:
        return c_srl(&B);
      case 0x7C:
        return c_bit(H, 7);
      case 0x7E:
        return c_bit(gt_HL(), 7);
      case 0x86:
        return c_res(&mem[gt_HL()], 0);
      case 0x87:
        return c_res(&A, 0);
      case 0x8E:
        return c_res(&mem[gt_HL()], 1);
      case 0x96:
        return c_res(&mem[gt_HL()], 2);
      case 0x9E:
        return c_res(&mem[gt_HL()], 3);
      case 0xA6:
        return c_res(&mem[gt_HL()], 4);
      case 0xAE:
        return c_res(&mem[gt_HL()], 5);
      case 0xB6:
        return c_res(&mem[gt_HL()], 6);
      case 0xC6:
        return c_set(&mem[gt_HL()], 0);
      case 0xCE:
        return c_set(&mem[gt_HL()], 1);
      case 0xD6:
        return c_set(&mem[gt_HL()], 2);
      case 0xDE:
        return c_set(&mem[gt_HL()], 3);
      case 0xE6:
        return c_set(&mem[gt_HL()], 4);
      case 0xEE:
        return c_set(&mem[gt_HL()], 5);
      case 0xF6:
        return c_set(&mem[gt_HL()], 6);
      case 0xFE:
        return c_set(&mem[gt_HL()], 7);
      default:
        printf("UNIMPLEMENTED PREFIX INSTRUCTION\n");
        return -1;
    }
  } else {
    switch(instr) {
      case 0x00:
        return 4;
      case 0x01:
        st_BC(rd16());
        return 12;
      case 0x03:
        st_BC(gt_BC() + 1);
        return 8;
      case 0x04:
        return c_inc(&B);
      case 0x05:
        return c_dec(&B);
      case 0x06:
        B = rd8();
        return 8;
      case 0x0A:
        A = r_mem(gt_BC());
        return 8;
      case 0x0B:
        st_BC(gt_BC() - 1);
        return 8;
      case 0x0C:
        return c_inc(&C);
      case 0x0D:
        return c_dec(&C);
      case 0x0E:
        C = rd8();
        return 8;
      case 0x11:
        st_DE(rd16());
        return 12;
      case 0x12:
        w_mem(gt_DE(), A);
        return 8;
      case 0x13:
        st_DE(gt_DE() + 1);
        return 8;
      case 0x14:
        return c_inc(&D);
      case 0x15:
        return c_dec(&D);
      case 0x16:
        D = rd8();
        return 8;
      case 0x17:
        return c_rl(&A);
      case 0x18:
        PC += (signed char) rd8();
        return 12;
      case 0x19:
        st_h_add16(gt_HL(), gt_DE());
        st_c_add16(gt_HL(), gt_DE());
        st_HL(gt_HL() + gt_DE());
        cl_flg(FLG_N);
        return 8;
      case 0x1A:
        A = r_mem(gt_DE());
        return 8;
      case 0x1D:
        return c_dec(&E);
      case 0x1E:
        E = rd8();
        return 8;
      case 0x1F:
        return c_rr(&A);
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
        return c_inc(&H);
      case 0x25:
        return c_dec(&H);
      case 0x26:
        H = rd8();
        return 8;
      case 0x28:
        if(gt_flg(FLG_Z)) {
          PC += (signed char) rd8();
          return 12;
        } else {
          rd8();
          return 8;
        }
      case 0x2A:
        A = r_mem(gt_HL());
        st_HL(gt_HL() + 1);
        return 8;
      case 0x2C:
        return c_inc(&L);
      case 0x2D:
        return c_dec(&L);
      case 0x2E:
        L = rd8();
        return 8;
      case 0x2F:
        return c_cpl(&A);
      case 0x30:
        if(!gt_flg(FLG_C)) {
          PC += (signed char) rd8();
          return 12;
        } else {
          rd8();
          return 8;
        }
      case 0x31:
        SP = rd16();
        return 12;
      case 0x32:
        w_mem(gt_HL(), A);
        st_HL(gt_HL() - 1);
        return 8;
      case 0x34:
        c_inc(&mem[gt_HL()]);
        return 12;
      case 0x35:
        return c_dec(&mem[gt_HL()]);
      case 0x36:
        w_mem(gt_HL(), rd8());
        return 12;
      case 0x3C:
        return c_inc(&A);
      case 0x3D:
        return c_dec(&A);
      case 0x3E:
        A = rd8();
        return 8;
      case 0x40:
        B = B;
        return 4;
      case 0x46:
        B = r_mem(gt_HL());
        return 8;
      case 0x47:
        B = A;
        return 4;
      case 0x4E:
        C = r_mem(gt_HL());
        return 8;
      case 0x4F:
        C = A;
        return 4;
      case 0x56:
        D = r_mem(gt_HL());
        return 8;
      case 0x57:
        D = A;
        return 4;
      case 0x5E:
        E = r_mem(gt_HL());
        return 8;
      case 0x5F:
        E = A;
        return 4;
      case 0x67:
        H = A;
        return 4;
      case 0x6F:
        L = A;
        return 4;
      case 0x70:
        w_mem(gt_HL(), B);
        return 8;
      case 0x71:
        w_mem(gt_HL(), C);
        return 8;
      case 0x72:
        w_mem(gt_HL(), D);
        return 8;
      case 0x76:
        return 4;
      case 0x77:
        w_mem(gt_HL(), A);
        return 8;
      case 0x78:
        A = B;
        return 4;
      case 0x79:
        A = C;
        return 4;
      case 0x7A:
        A = D;
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
      case 0x7E:
        A = r_mem(gt_HL());
        return 8;
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
      case 0x87:
        return c_add(A);
      case 0x90:
        st_h_sub(A, B);
        st_c_sub(A, B);
        A -= B;
        st_z(A);
        st_flg(FLG_N);
        return 4;
      case 0xA1:
        return c_and(C);
      case 0xA7:
        return c_and(A);
      case 0xA9:
        return c_xor(C);
      case 0xAE:
        return c_xor(r_mem(gt_HL()));
      case 0xAF:
        A = 0;
        st_flg(FLG_Z);
        cl_flg(FLG_N);
        cl_flg(FLG_H);
        cl_flg(FLG_C);
        return 4;
      case 0xB0:
        return c_or(B);
      case 0xB1:
        return c_or(C);
      case 0xB7:
        return c_or(A);
      case 0xBE:
        {
          byte var = r_mem(gt_HL());
          st_z(A - var);
          st_flg(FLG_N);
          st_h_sub(A, var);
          st_c_sub(A, var);
          return 8;
        }
      case 0xC0:
        if(!gt_flg(FLG_Z)) {
          PC = pop16();
          kp();
          return 20;
        } else return 8;
      case 0xC1:
        st_BC(pop16());
        return 12;
      case 0xC3:
        PC = rd16();
        kp();
        return 16;
      case 0xC4:
        if(!gt_flg(FLG_Z)) {
          PC = rd16();
          kp();
          return 24;
        } else {
          rd16();
          return 12;
        }
      case 0xC5:
        psh16(gt_BC());
        return 16;
      case 0xC6:
        return c_add(rd8());
      case 0xC8:
        if(gt_flg(FLG_Z)) {
          PC = pop16();
          kp();
          return 20;
        } else return 8;
      case 0xC9:
        PC = pop16();
        kp();
        return 16;
      case 0xCA:
        if(gt_flg(FLG_Z)) {
          PC = rd16();
          kp();
          return 16;
        } else rd16();
        return 12;
      case 0xCD:
        psh16(PC + 3);
        PC = rd16();
        kp();
        return 24;
      case 0xD1:
        st_DE(pop16());
        return 12;
      case 0xD5:
        psh16(gt_DE());
        return 16;
      case 0xD6:
        return c_and(rd8());
      case 0xD9:
        PC = pop16();
        kp();
        IME = 1;
        return 16;
      case 0xE0:
        w_mem(0xFF00 + rd8(), A);
        return 12;
      case 0xE1:
        st_HL(pop16());
        return 12;
      case 0xE2:
        w_mem(0xFF00 + C, A);
        return 8;
      case 0xE5:
        psh16(gt_HL());
        return 16;
      case 0xE6:
        return c_and(rd8());
      case 0xE9:
        PC = gt_HL();
        kp();
        return 4;
      case 0xEA:
        w_mem(rd16(), A);
        return 16;
      case 0xEE:
        return c_xor(rd8());
      case 0xEF:
        return c_rst(0x0028);
      case 0xF0:
        A = r_mem(0xFF00 + rd8());
        return 12;
      case 0xF3:
        IME = 0;
        return 4;
      case 0xF1:
        st_AF(pop16());
        return 12;
      case 0xF5:
        psh16(gt_AF());
        return 16;
      case 0xFA:
        A = rd16();
        return 16;
      case 0xFB:
        IME = 1;
        return 4;
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
