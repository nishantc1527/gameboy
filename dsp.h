#ifndef DEF
#define DEF

#include <stdio.h>

#include <SDL.h>

#include "var.h"
#include "func.h"

void init_dsp() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("ERROR CREATING WINDOW: %s\n", SDL_GetError());
        exit(1);
    }
    win = SDL_CreateWindow("GAMEBOY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * FCT, 144 * FCT, 0);
    rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
}

void upd_stat() {
    byte stat = LCD_STAT;
    int curr = stat & 0b11;
    int mode;
    if (scn >= 456 - 80) mode = 2;
    else if (scn >= 456 - 80 - 168) mode = 3;
    else mode = 0;
    if (LY >= 144) mode = 1;
    if (mode != curr && mode == 1) {
        req_intr(0);
        if (gt_bt(stat, 4)) req_intr(1);
    }
    if ((mode == 0 && curr != 0 && gt_bt(stat, 3)) || (mode == 2 && curr != 2 && gt_bt(stat, 5))) {
        req_intr(0);
    }
    stat &= ~(0b11);
    stat |= mode;

    curr = gt_bt(stat, 2);

    if (LY == LYC) st_bt(&stat, 2);
    else cl_bt(&stat, 2);
    if (gt_bt(stat, 2) != curr && gt_bt(stat, 2) && gt_bt(stat, 6)) req_intr(1);
    w_mem(0xFF41, stat);
}

void scnln() {
    if (gt_bt(LCDC, 7)) {
        if (LY < SCRN_HEIGHT) {
            if (gt_bt(LCDC, 0)) {
                byte ly = LY;
                int dat_area = gt_bt(LCDC, 4);
                int mp_area = gt_bt(LCDC, 3);
                byte pal = BGP;
                for (int x = 0; x < SCRN_WIDTH; x++) {
                    byte by = (ly + SCY) % 256;
                    int tiley = by / 8;
                    int offy = by % 8;
                    int bytey = offy << 1;
                    int bx = (x + SCX) % 256;
                    int tilex = bx / 8;
                    int offx = bx % 8;
                    int idx = (tiley * 32) + tilex;
                    if (mp_area == 0) idx += 0x9800;
                    else idx += 0x9C00;
                    idx = r_mem(idx);
                    if (dat_area == 0) idx = (signed char) idx + 128;
                    idx *= 16;
                    if (dat_area == 0) idx += 0x8800;
                    else idx += 0x8000;
                    byte ls = r_mem(idx + bytey);
                    byte ms = r_mem(idx + bytey + 1);
                    offx = 7 - offx;
                    int clr = (gt_bt(ms, offx) << 1) | gt_bt(ls, offx);
                    dsp[ly][x] = gt_clr(pal, clr);
                }
                if (gt_bt(LCDC, 5)) {
                    mp_area = gt_bt(LCDC, 6);
                    byte wx = WX;
                    byte wy = WY;
                    if (wx >= 0 && wx <= 166 && wy >= 0 && wy <= 143 && LY >= wy) {
                        wx = wx - 7;
                        wy = WIN_CNT;
                        int tiley = wy / 8;
                        int offy = wy % 8;
                        int bytey = offy << 1;
                        for (byte x = wx; x < SCRN_WIDTH; x++) {
                            byte _wx = x - wx;
                            int tilex = _wx / 8;
                            int offx = _wx % 8;
                            int idx = (tiley * 32) + tilex;
                            if (mp_area == 0) idx += 0x9800;
                            else idx += 0x9C00;
                            idx = r_mem(idx);
                            if (dat_area == 0) idx = (signed char)idx + 128;
                            idx *= 16;
                            if (dat_area == 0) idx += 0x8800;
                            else idx += 0x8000;
                            byte ls = r_mem(idx + bytey);
                            byte ms = r_mem(idx + bytey + 1);
                            offx = 7 - offx;
                            int clr = (gt_bt(ms, offx) << 1) | gt_bt(ls, offx);
                            dsp[ly][x] = gt_clr(pal, clr);
                        }
                        WIN_CNT++;
                    }
                }
            }
            else {
                for (int x = 0; x < SCRN_WIDTH; x++) {
                    dsp[LY][x] = gt_clr(BGP, 0);
                }
            }
            if (gt_bt(LCDC, 1)) {
                byte ly = LY;
                byte sz = gt_bt(LCDC, 2);
                int cnt = 0;
                int vis[1000];
                memset(vis, 0, sizeof(vis));
                for (dbyte mem = 0xFE00; mem <= 0xFE9F && cnt < 10; mem += 4) {
                    signed char y   = r_mem(mem + 0);
                    signed char x   = r_mem(mem + 1);
                    dbyte idx = r_mem(mem + 2);
                    byte flg = r_mem(mem + 3);
                    y -= 16;
                    x -= 8;
                    if (ly < y) continue;
                    if (sz) {
                        if (ly >= y + 16) continue;
                        else idx &= 0xFE;
                    }
                    else if (ly >= y + 8) continue;
                    if (vis[x]) continue;
                    vis[x] = 1;
                    cnt++;
                    idx *= 16;
                    idx += 0x8000;
                    byte flipx = gt_bt(flg, 5);
                    byte flipy = gt_bt(flg, 6);
                    byte line = ly - y;
                    if (sz) {
                        if (flipy) line = 15 - line;
                    }
                    else {
                        if (flipy) line = 7 - line;
                    }
                    line <<= 1;
                    byte ls = r_mem(idx + line + 0);
                    byte ms = r_mem(idx + line + 1);
                    byte pal;
                    if (gt_bt(flg, 4)) pal = OBP1;
                    else pal = OBP0;
                    for (signed char x0 = x; x0 < x + 8; x0++) {
                        if (x0 < 0) continue;
                        byte posx = 7 - (x0 - x);
                        if (flipx) posx = 7 - posx;
                        byte clr = (gt_bt(ms, posx) << 1) | gt_bt(ls, posx);
                        if (gt_bt(flg, 7)) {
                            if (dsp[ly][x0] == gt_clr(BGP, 0)) dsp[ly][x0] = gt_clr(pal, clr);
                        }
                        else if (clr != 0) dsp[ly][x0] = gt_clr(pal, clr);
                    }
                }
            }
        }
        int ly = LY;
        ly++;
        if (ly >= 154) {
            ly = 0;
            WIN_CNT = 0;
            frame = 1;
        }
        w_mem(0xFF44, ly);
    }
}

void drw_screen() {
    for (int i = 0; i < SCRN_HEIGHT; i++) {
        for (int j = 0; j < SCRN_WIDTH; j++) {
            int clr = dsp[i][j];
            if (clr == CLR_WHT) clr = HEX_WHT;
            else if (clr == CLR_L_GRY) clr = HEX_L_GREY;
            else if (clr == CLR_D_GRY) clr = HEX_R_GREY;
            else if (clr == CLR_BLK) clr = HEX_BLK;
            SDL_SetRenderDrawColor(rnd, clr, clr, clr, 0xFF);
            SDL_Rect rct;
            rct.x = j * FCT;
            rct.y = i * FCT;
            rct.w = FCT;
            rct.h = FCT;
            SDL_RenderFillRect(rnd, &rct);
        }
    }
    SDL_RenderPresent(rnd);
}

int rndr() {
    drw_screen();
    return 0;
}

#endif
