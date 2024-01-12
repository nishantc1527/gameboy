#ifndef CPU
#define CPU

#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"

int exec() {
    int dis = 0;
    byte instr = r_mem(PC);
    if (PC >= 0x8000 && 0) { // Unset for debugging
        printf("PROGRAM COUNTER PAST CARTRIDGE SPACE\n");
        return -1;
    }
    if (instr == 0xCB) {
        byte prfx = rd8();
        if (dis) p_instr(instr, prfx);
        switch (prfx) {
        case 0x00:
            return c_rlc(&B);
        case 0x01:
            return c_rlc(&C);
        case 0x02:
            return c_rlc(&D);
        case 0x03:
            return c_rlc(&E);
        case 0x04:
            return c_rlc(&H);
        case 0x05:
            return c_rlc(&L);
        case 0x06:
            return c_rlc(&mem[gt_HL()]);
        case 0x07:
            return c_rlc(&A);
        case 0x08:
            return c_rrc(&B);
        case 0x09:
            return c_rrc(&C);
        case 0x0A:
            return c_rrc(&D);
        case 0x0B:
            return c_rrc(&E);
        case 0x0C:
            return c_rrc(&H);
        case 0x0D:
            return c_rrc(&L);
        case 0x0E:
            return c_rrc(&mem[gt_HL()]);
        case 0x0F:
            return c_rrc(&A);
        case 0x10:
            return c_rl(&B);
        case 0x11:
            return c_rl(&C);
        case 0x12:
            return c_rl(&D);
        case 0x13:
            return c_rl(&E);
        case 0x14:
            return c_rl(&H);
        case 0x15:
            return c_rl(&L);
        case 0x16:
            return c_rl(&mem[gt_HL()]);
        case 0x17:
            return c_rl(&A);
        case 0x18:
            return c_rr(&B);
        case 0x19:
            return c_rr(&C);
        case 0x1A:
            return c_rr(&D);
        case 0x1B:
            return c_rr(&E);
        case 0x1C:
            return c_rr(&H);
        case 0x1D:
            return c_rr(&L);
        case 0x1E:
            return c_rr(&mem[gt_HL()]);
        case 0x1F:
            return c_rr(&A);
        case 0x20:
            return c_sla(&B);
        case 0x21:
            return c_sla(&C);
        case 0x22:
            return c_sla(&D);
        case 0x23:
            return c_sla(&E);
        case 0x24:
            return c_sla(&H);
        case 0x25:
            return c_sla(&L);
        case 0x26:
            return c_sla(&mem[gt_HL()]);
        case 0x27:
            return c_sla(&A);
        case 0x28:
            return c_sra(&B);
        case 0x29:
            return c_sra(&C);
        case 0x2A:
            return c_sra(&D);
        case 0x2B:
            return c_sra(&E);
        case 0x2C:
            return c_sra(&H);
        case 0x2D:
            return c_sra(&L);
        case 0x2E:
            return c_sra(&mem[gt_HL()]);
        case 0x2F:
            return c_sra(&A);
        case 0x30:
            return c_swp(&B);
        case 0x31:
            return c_swp(&C);
        case 0x32:
            return c_swp(&D);
        case 0x33:
            return c_swp(&E);
        case 0x34:
            return c_swp(&H);
        case 0x35:
            return c_swp(&L);
        case 0x36:
            return c_swp(&mem[gt_HL()]);
        case 0x37:
            return c_swp(&A);
        case 0x38:
            return c_srl(&B);
        case 0x39:
            return c_srl(&C);
        case 0x3A:
            return c_srl(&D);
        case 0x3B:
            return c_srl(&E);
        case 0x3C:
            return c_srl(&H);
        case 0x3D:
            return c_srl(&L);
        case 0x3E:
            return c_srl(&mem[gt_HL()]);
        case 0x3F:
            return c_srl(&A);
        case 0x40:
            return c_bit(B, 0);
        case 0x41:
            return c_bit(C, 0);
        case 0x42:
            return c_bit(D, 0);
        case 0x43:
            return c_bit(E, 0);
        case 0x44:
            return c_bit(H, 0);
        case 0x45:
            return c_bit(L, 0);
        case 0x46:
            return c_bit(r_mem(gt_HL()), 0);
        case 0x47:
            return c_bit(A, 0);
        case 0x48:
            return c_bit(B, 1);
        case 0x49:
            return c_bit(C, 1);
        case 0x4A:
            return c_bit(D, 1);
        case 0x4B:
            return c_bit(E, 1);
        case 0x4C:
            return c_bit(H, 1);
        case 0x4D:
            return c_bit(L, 1);
        case 0x4E:
            return c_bit(r_mem(gt_HL()), 1);
        case 0x4F:
            return c_bit(A, 1);
        case 0x50:
            return c_bit(B, 2);
        case 0x51:
            return c_bit(C, 2);
        case 0x52:
            return c_bit(D, 2);
        case 0x53:
            return c_bit(E, 2);
        case 0x54:
            return c_bit(H, 2);
        case 0x55:
            return c_bit(L, 2);
        case 0x56:
            return c_bit(r_mem(gt_HL()), 2);
        case 0x57:
            return c_bit(A, 2);
        case 0x58:
            return c_bit(B, 3);
        case 0x59:
            return c_bit(C, 3);
        case 0x5A:
            return c_bit(D, 3);
        case 0x5B:
            return c_bit(E, 3);
        case 0x5C:
            return c_bit(H, 3);
        case 0x5D:
            return c_bit(L, 3);
        case 0x5E:
            return c_bit(r_mem(gt_HL()), 3);
        case 0x5F:
            return c_bit(A, 3);
        case 0x60:
            return c_bit(B, 4);
        case 0x61:
            return c_bit(C, 4);
        case 0x62:
            return c_bit(D, 4);
        case 0x63:
            return c_bit(E, 4);
        case 0x64:
            return c_bit(H, 4);
        case 0x65:
            return c_bit(L, 4);
        case 0x66:
            return c_bit(r_mem(gt_HL()), 4);
        case 0x67:
            return c_bit(A, 4);
        case 0x68:
            return c_bit(B, 5);
        case 0x69:
            return c_bit(C, 5);
        case 0x6A:
            return c_bit(D, 5);
        case 0x6B:
            return c_bit(E, 5);
        case 0x6C:
            return c_bit(H, 5);
        case 0x6D:
            return c_bit(L, 5);
        case 0x6E:
            return c_bit(r_mem(gt_HL()), 5);
        case 0x6F:
            return c_bit(A, 5);
        case 0x70:
            return c_bit(B, 6);
        case 0x71:
            return c_bit(C, 6);
        case 0x72:
            return c_bit(D, 6);
        case 0x73:
            return c_bit(E, 6);
        case 0x74:
            return c_bit(H, 6);
        case 0x75:
            return c_bit(L, 6);
        case 0x76:
            return c_bit(r_mem(gt_HL()), 6);
        case 0x77:
            return c_bit(A, 6);
        case 0x78:
            return c_bit(B, 7);
        case 0x79:
            return c_bit(C, 7);
        case 0x7A:
            return c_bit(D, 7);
        case 0x7B:
            return c_bit(E, 7);
        case 0x7C:
            return c_bit(H, 7);
        case 0x7D:
            return c_bit(L, 7);
        case 0x7E:
            return c_bit(r_mem(gt_HL()), 7);
        case 0x7F:
            return c_bit(A, 7);
        case 0x80:
            return c_res(&B, 0);
        case 0x81:
            return c_res(&C, 0);
        case 0x82:
            return c_res(&D, 0);
        case 0x83:
            return c_res(&E, 0);
        case 0x84:
            return c_res(&H, 0);
        case 0x85:
            return c_res(&L, 0);
        case 0x86:
            return c_res(&mem[gt_HL()], 0);
        case 0x87:
            return c_res(&A, 0);
        case 0x88:
            return c_res(&B, 1);
        case 0x89:
            return c_res(&C, 1);
        case 0x8A:
            return c_res(&D, 1);
        case 0x8B:
            return c_res(&E, 1);
        case 0x8C:
            return c_res(&H, 1);
        case 0x8D:
            return c_res(&L, 1);
        case 0x8E:
            return c_res(&mem[gt_HL()], 1);
        case 0x8F:
            return c_res(&A, 1);
        case 0x90:
            return c_res(&B, 2);
        case 0x91:
            return c_res(&C, 2);
        case 0x92:
            return c_res(&D, 2);
        case 0x93:
            return c_res(&E, 2);
        case 0x94:
            return c_res(&H, 2);
        case 0x95:
            return c_res(&L, 2);
        case 0x96:
            return c_res(&mem[gt_HL()], 2);
        case 0x97:
            return c_res(&A, 2);
        case 0x98:
            return c_res(&B, 3);
        case 0x99:
            return c_res(&C, 3);
        case 0x9A:
            return c_res(&D, 3);
        case 0x9B:
            return c_res(&E, 3);
        case 0x9C:
            return c_res(&H, 3);
        case 0x9D:
            return c_res(&L, 3);
        case 0x9E:
            return c_res(&mem[gt_HL()], 3);
        case 0x9F:
            return c_res(&A, 3);
        case 0xA0:
            return c_res(&B, 4);
        case 0xA1:
            return c_res(&C, 4);
        case 0xA2:
            return c_res(&D, 4);
        case 0xA3:
            return c_res(&E, 4);
        case 0xA4:
            return c_res(&H, 4);
        case 0xA5:
            return c_res(&L, 4);
        case 0xA6:
            return c_res(&mem[gt_HL()], 4);
        case 0xA7:
            return c_res(&A, 4);
        case 0xA8:
            return c_res(&B, 5);
        case 0xA9:
            return c_res(&C, 5);
        case 0xAA:
            return c_res(&D, 5);
        case 0xAB:
            return c_res(&E, 5);
        case 0xAC:
            return c_res(&H, 5);
        case 0xAD:
            return c_res(&L, 5);
        case 0xAE:
            return c_res(&mem[gt_HL()], 5);
        case 0xAF:
            return c_res(&A, 5);
        case 0xB0:
            return c_res(&B, 6);
        case 0xB1:
            return c_res(&C, 6);
        case 0xB2:
            return c_res(&D, 6);
        case 0xB3:
            return c_res(&E, 6);
        case 0xB4:
            return c_res(&H, 6);
        case 0xB5:
            return c_res(&L, 6);
        case 0xB6:
            return c_res(&mem[gt_HL()], 6);
        case 0xB7:
            return c_res(&A, 6);
        case 0xB8:
            return c_res(&B, 7);
        case 0xB9:
            return c_res(&C, 7);
        case 0xBA:
            return c_res(&D, 7);
        case 0xBB:
            return c_res(&E, 7);
        case 0xBC:
            return c_res(&H, 7);
        case 0xBD:
            return c_res(&L, 7);
        case 0xBE:
            return c_res(&mem[gt_HL()], 7);
        case 0xBF:
            return c_res(&A, 7);
        case 0xC0:
            return c_set(&B, 0);
        case 0xC1:
            return c_set(&C, 0);
        case 0xC2:
            return c_set(&D, 0);
        case 0xC3:
            return c_set(&E, 0);
        case 0xC4:
            return c_set(&H, 0);
        case 0xC5:
            return c_set(&L, 0);
        case 0xC6:
            return c_set(&mem[gt_HL()], 0);
        case 0xC7:
            return c_set(&A, 0);
        case 0xC8:
            return c_set(&B, 1);
        case 0xC9:
            return c_set(&C, 1);
        case 0xCA:
            return c_set(&D, 1);
        case 0xCB:
            return c_set(&E, 1);
        case 0xCC:
            return c_set(&H, 1);
        case 0xCD:
            return c_set(&L, 1);
        case 0xCE:
            return c_set(&mem[gt_HL()], 1);
        case 0xCF:
            return c_set(&A, 1);
        case 0xD0:
            return c_set(&B, 2);
        case 0xD1:
            return c_set(&C, 2);
        case 0xD2:
            return c_set(&D, 2);
        case 0xD3:
            return c_set(&E, 2);
        case 0xD4:
            return c_set(&H, 2);
        case 0xD5:
            return c_set(&L, 2);
        case 0xD6:
            return c_set(&mem[gt_HL()], 2);
        case 0xD7:
            return c_set(&A, 2);
        case 0xD8:
            return c_set(&B, 3);
        case 0xD9:
            return c_set(&C, 3);
        case 0xDA:
            return c_set(&D, 3);
        case 0xDB:
            return c_set(&E, 3);
        case 0xDC:
            return c_set(&H, 3);
        case 0xDD:
            return c_set(&L, 3);
        case 0xDE:
            return c_set(&mem[gt_HL()], 3);
        case 0xDF:
            return c_set(&A, 3);
        case 0xE0:
            return c_set(&B, 4);
        case 0xE1:
            return c_set(&C, 4);
        case 0xE2:
            return c_set(&D, 4);
        case 0xE3:
            return c_set(&E, 4);
        case 0xE4:
            return c_set(&H, 4);
        case 0xE5:
            return c_set(&L, 4);
        case 0xE6:
            return c_set(&mem[gt_HL()], 4);
        case 0xE7:
            return c_set(&A, 4);
        case 0xE8:
            return c_set(&B, 5);
        case 0xE9:
            return c_set(&C, 5);
        case 0xEA:
            return c_set(&D, 5);
        case 0xEB:
            return c_set(&E, 5);
        case 0xEC:
            return c_set(&H, 5);
        case 0xED:
            return c_set(&L, 5);
        case 0xEE:
            return c_set(&mem[gt_HL()], 5);
        case 0xEF:
            return c_set(&A, 5);
        case 0xF0:
            return c_set(&B, 6);
        case 0xF1:
            return c_set(&C, 6);
        case 0xF2:
            return c_set(&D, 6);
        case 0xF3:
            return c_set(&E, 6);
        case 0xF4:
            return c_set(&H, 6);
        case 0xF5:
            return c_set(&L, 6);
        case 0xF6:
            return c_set(&mem[gt_HL()], 6);
        case 0xF7:
            return c_set(&A, 6);
        case 0xF8:
            return c_set(&B, 7);
        case 0xF9:
            return c_set(&C, 7);
        case 0xFA:
            return c_set(&D, 7);
        case 0xFB:
            return c_set(&E, 7);
        case 0xFC:
            return c_set(&H, 7);
        case 0xFD:
            return c_set(&L, 7);
        case 0xFE:
            return c_set(&mem[gt_HL()], 7);
        case 0xFF:
            return c_set(&A, 7);
        default:
            printf("UNIMPLEMENTED PREFIX INSTRUCTION\n");
            p_instr(instr, prfx);
            return -1;
        }
    }
    else {
        if (dis) p_instr(instr, 0);
        switch (instr) {
        case 0x00:
            return 4;
        case 0x01:
            st_BC(rd16());
            return 12;
        case 0x02:
            w_mem(gt_BC(), A);
            return 8;
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
        case 0x07:
            c_rlc(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x08:
        {
            dbyte addr = rd16();
            w_mem(addr, (byte)(SP & 0xFF));
            w_mem(addr + 1, (byte)((SP >> 8) & 0xFF));
            return 20;
        }
        case 0x09:
            st_h_add16(gt_HL(), gt_BC());
            st_c_add16(gt_HL(), gt_BC());
            st_HL(gt_HL() + gt_BC());
            cl_flg(FLG_N);
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
        case 0x0F:
            c_rrc(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x10:
            return 4;
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
            c_rl(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x18:
            PC += (signed char)rd8();
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
        case 0x1B:
            st_DE(gt_DE() - 1);
            return 8;
        case 0x1C:
            return c_inc(&E);
        case 0x1D:
            return c_dec(&E);
        case 0x1E:
            E = rd8();
            return 8;
        case 0x1F:
            c_rr(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x20:
            return c_jp8(1 - gt_flg(FLG_Z));
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
        case 0x27: // No clue how this works I just copied it
            if (!gt_flg(FLG_N)) {
                if (gt_flg(FLG_C) || A > 0x99) {
                    A += 0x60;
                    st_flg(FLG_C);
                }
                if (gt_flg(FLG_H) || (A & 0x0f) > 0x09) {
                    A += 0x06;
                }
            }
            else {
                if (gt_flg(FLG_C)) {
                    A -= 0x60;
                }
                if (gt_flg(FLG_H)) {
                    A -= 0x06;
                }
            }
            st_z(A);
            cl_flg(FLG_H);
            return 4;
        case 0x28:
            return c_jp8(gt_flg(FLG_Z));
        case 0x29:
            st_h_add16(gt_HL(), gt_HL());
            st_c_add16(gt_HL(), gt_HL());
            st_HL(gt_HL() + gt_HL());
            cl_flg(FLG_N);
            return 8;
        case 0x2A:
            A = r_mem(gt_HL());
            st_HL(gt_HL() + 1);
            return 8;
        case 0x2B:
            st_HL(gt_HL() - 1);
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
            return c_jp8(1 - gt_flg(FLG_C));
        case 0x31:
            SP = rd16();
            return 12;
        case 0x32:
            w_mem(gt_HL(), A);
            st_HL(gt_HL() - 1);
            return 8;
        case 0x33:
            ++SP;
            return 8;
        case 0x34:
            return c_inc(&mem[gt_HL()]);
        case 0x35:
            return c_dec(&mem[gt_HL()]);
        case 0x36:
            w_mem(gt_HL(), rd8());
            return 12;
        case 0x37:
            cl_flg(FLG_N);
            cl_flg(FLG_H);
            st_flg(FLG_C);
            return 4;
        case 0x38:
            return c_jp8(gt_flg(FLG_C));
        case 0x39:
            st_h_add16(gt_HL(), SP);
            st_c_add16(gt_HL(), SP);
            st_HL(gt_HL() + SP);
            cl_flg(FLG_N);
            return 8;
        case 0x3A:
            A = r_mem(gt_HL());
            st_HL(gt_HL() - 1);
            return 8;
        case 0x3B:
            --SP;
            return 8;
        case 0x3C:
            return c_inc(&A);
        case 0x3D:
            return c_dec(&A);
        case 0x3E:
            A = rd8();
            return 8;
        case 0x3F:
            cl_flg(FLG_N);
            cl_flg(FLG_H);
            if (gt_flg(FLG_C)) cl_flg(FLG_C);
            else st_flg(FLG_C);
            return 4;
        case 0x40:
            B = B;
            return 4;
        case 0x41:
            B = C;
            return 4;
        case 0x42:
            B = D;
            return 4;
        case 0x43:
            B = E;
            return 4;
        case 0x44:
            B = H;
            return 4;
        case 0x45:
            B = L;
            return 4;
        case 0x46:
            B = r_mem(gt_HL());
            return 8;
        case 0x47:
            B = A;
            return 4;
        case 0x48:
            C = B;
            return 4;
        case 0x49:
            C = C;
            return 4;
        case 0x4A:
            C = D;
            return 4;
        case 0x4B:
            C = E;
            return 4;
        case 0x4C:
            C = H;
            return 4;
        case 0x4D:
            C = L;
            return 4;
        case 0x4E:
            C = r_mem(gt_HL());
            return 8;
        case 0x4F:
            C = A;
            return 4;
        case 0x50:
            D = B;
            return 4;
        case 0x51:
            D = C;
            return 4;
        case 0x52:
            D = D;
            return 4;
        case 0x53:
            D = E;
            return 4;
        case 0x54:
            D = H;
            return 4;
        case 0x55:
            D = L;
            return 4;
        case 0x56:
            D = r_mem(gt_HL());
            return 8;
        case 0x57:
            D = A;
            return 4;
        case 0x58:
            E = B;
            return 4;
        case 0x59:
            E = C;
            return 4;
        case 0x5A:
            E = D;
            return 4;
        case 0x5B:
            E = E;
            return 4;
        case 0x5C:
            E = H;
            return 4;
        case 0x5D:
            E = L;
            return 4;
        case 0x5E:
            E = r_mem(gt_HL());
            return 8;
        case 0x5F:
            E = A;
            return 4;
        case 0x60:
            H = B;
            return 4;
        case 0x61:
            H = C;
            return 4;
        case 0x62:
            H = D;
            return 4;
        case 0x63:
            H = E;
            return 4;
        case 0x64:
            H = H;
            return 4;
        case 0x65:
            H = L;
            return 4;
        case 0x66:
            H = r_mem(gt_HL());
            return 8;
        case 0x67:
            H = A;
            return 4;
        case 0x68:
            L = B;
            return 4;
        case 0x69:
            L = C;
            return 4;
        case 0x6A:
            L = D;
            return 4;
        case 0x6B:
            L = E;
            return 4;
        case 0x6C:
            L = H;
            return 4;
        case 0x6D:
            L = L;
            return 4;
        case 0x6E:
            L = r_mem(gt_HL());
            return 8;
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
        case 0x73:
            w_mem(gt_HL(), E);
            return 8;
        case 0x74:
            w_mem(gt_HL(), H);
            return 8;
        case 0x75:
            w_mem(gt_HL(), L);
            return 8;
        case 0x76:
            HALT = 1;
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
        case 0x7F:
            A = A;
            return 4;
        case 0x80:
            return c_add(B);
        case 0x81:
            return c_add(C);
        case 0x82:
            return c_add(D);
        case 0x83:
            return c_add(E);
        case 0x84:
            return c_add(H);
        case 0x85:
            return c_add(L);
        case 0x86:
            return c_add(r_mem(gt_HL()));
        case 0x87:
            return c_add(A);
        case 0x88:
            return c_adc(B);
        case 0x89:
            return c_adc(C);
        case 0x8A:
            return c_adc(D);
        case 0x8B:
            return c_adc(E);
        case 0x8C:
            return c_adc(H);
        case 0x8D:
            return c_adc(L);
        case 0x8E:
            return c_adc(r_mem(gt_HL()));
        case 0x8F:
            return c_adc(A);
        case 0x90:
            return c_sub(B);
        case 0x91:
            return c_sub(C);
        case 0x92:
            return c_sub(D);
        case 0x93:
            return c_sub(E);
        case 0x94:
            return c_sub(H);
        case 0x95:
            return c_sub(L);
        case 0x96:
            return c_sub(r_mem(gt_HL()));
        case 0x97:
            return c_sub(A);
        case 0x98:
            return c_sbc(B);
        case 0x99:
            return c_sbc(C);
        case 0x9A:
            return c_sbc(D);
        case 0x9B:
            return c_sbc(E);
        case 0x9C:
            return c_sbc(H);
        case 0x9D:
            return c_sbc(L);
        case 0x9E:
            return c_sbc(r_mem(gt_HL()));
        case 0x9F:
            return c_sbc(A);
        case 0xA0:
            return c_and(B);
        case 0xA1:
            return c_and(C);
        case 0xA2:
            return c_and(D);
        case 0xA3:
            return c_and(E);
        case 0xA4:
            return c_and(H);
        case 0xA5:
            return c_and(L);
        case 0xA6:
            return c_and(r_mem(gt_HL()));
        case 0xA7:
            return c_and(A);
        case 0xA8:
            return c_xor(B);
        case 0xA9:
            return c_xor(C);
        case 0xAA:
            return c_xor(D);
        case 0xAB:
            return c_xor(E);
        case 0xAC:
            return c_xor(H);
        case 0xAD:
            return c_xor(L);
        case 0xAE:
            return c_xor(r_mem(gt_HL()));
        case 0xAF:
            return c_xor(A);
        case 0xB0:
            return c_or(B);
        case 0xB1:
            return c_or(C);
        case 0xB2:
            return c_or(D);
        case 0xB3:
            return c_or(E);
        case 0xB4:
            return c_or(H);
        case 0xB5:
            return c_or(L);
        case 0xB6:
            return c_or(r_mem(gt_HL()));
        case 0xB7:
            return c_or(A);
        case 0xB8:
            return c_cp(B);
        case 0xB9:
            return c_cp(C);
        case 0xBA:
            return c_cp(D);
        case 0xBB:
            return c_cp(E);
        case 0xBC:
            return c_cp(H);
        case 0xBD:
            return c_cp(L);
        case 0xBE:
            return c_cp(r_mem(gt_HL()));
        case 0xBF:
            return c_cp(A);
        case 0xC0:
            return c_ret(1 - gt_flg(FLG_Z));
        case 0xC1:
            st_BC(pop16());
            return 12;
        case 0xC2:
            return c_jp16(1 - gt_flg(FLG_Z));
        case 0xC3:
            PC = rd16();
            kp();
            return 16;
        case 0xC4:
            return c_call(1 - gt_flg(FLG_Z));
        case 0xC5:
            psh16(gt_BC());
            return 16;
        case 0xC6:
            return c_add(rd8());
        case 0xC7:
            return c_rst(0x0000);
        case 0xC8:
            return c_ret(gt_flg(FLG_Z));
        case 0xC9:
            return c_ret(1);
        case 0xCA:
            return c_jp16(gt_flg(FLG_Z));
        case 0xCC:
            return c_call(gt_flg(FLG_Z));
        case 0xCD:
            return c_call(1);
        case 0xCE:
            return c_adc(rd8());
        case 0xCF:
            return c_rst(0x0008);
        case 0xD0:
            return c_ret(1 - gt_flg(FLG_C));
        case 0xD1:
            st_DE(pop16());
            return 12;
        case 0xD2:
            return c_jp16(1 - gt_flg(FLG_C));
        case 0xD4:
            return c_call(1 - gt_flg(FLG_C));
        case 0xD5:
            psh16(gt_DE());
            return 16;
        case 0xD6:
            return c_sub(rd8());
        case 0xD7:
            return c_rst(0x0010);
        case 0xD8:
            return c_ret(gt_flg(FLG_C));
        case 0xD9:
            IME = 1;
            return c_ret(1);
        case 0xDA:
            return c_jp16(gt_flg(FLG_C));
        case 0xDC:
            return c_call(gt_flg(FLG_C));
        case 0xDE:
            return c_sbc(rd8());
        case 0xDF:
            return c_rst(0x0018);
        case 0xE0:
            w_mem(0xFF00 + (dbyte)rd8(), A);
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
        case 0xE7:
            return c_rst(0x0020);
        case 0xE8: // TODO half carry + carry flag
        {
            byte add = rd8();
            st_h_add((byte)SP, add);
            st_c_add((byte)SP, add);
            SP += (signed char)add;
            cl_flg(FLG_Z);
            cl_flg(FLG_N);
            return 16;
        }
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
            A = r_mem(0xFF00 + (dbyte)rd8());
            return 12;
        case 0xF1:
            st_AF(pop16() & 0xFFF0);
            return 12;
        case 0xF2:
            A = r_mem(0xFF00 + C);
            return 8;
        case 0xF3:
            IME = 0;
            return 4;
        case 0xF5:
            psh16(gt_AF());
            return 16;
        case 0xF6:
            return c_or(rd8());
        case 0xF7:
            return c_rst(0x0030);
        case 0xF8:
        {
            byte nxt = rd8();
            dbyte add = SP + (signed char)nxt;
            st_HL(add);
            cl_flg(FLG_Z);
            cl_flg(FLG_N);
            st_h_add((byte)SP, nxt);
            st_c_add((byte)SP, nxt);
            return 12;
        }
        case 0xF9:
            SP = gt_HL();
            return 8;
        case 0xFA:
            A = r_mem(rd16());
            return 16;
        case 0xFB:
            IME = 1;
            return 4;
        case 0xFE:
            return c_cp(rd8());
        case 0xFF:
            return c_rst(0x0038);
        default:
            printf("UNIMPLEMENTED INSTRUCTION\n");
            p_instr(instr, 0);
            return -1;
        }
    }
    return 0;
}

#endif
