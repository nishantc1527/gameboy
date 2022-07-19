#ifndef DEF
#define DEF

#include <stdio.h>

#include <SDL2/SDL.h>

#include "var.h"
#include "func.h"

void init_dsp() {
  if(SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("ERROR CREATING WINDOW: %s\n", SDL_GetError());
    exit(1);
  }

  win = SDL_CreateWindow("GAMEBOY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * FCT, 144 * FCT, 0);
  rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  tile_dat = SDL_CreateWindow("TILE DATA", 150, 200, 128 * FCT_DAT, 192 * FCT_DAT, 0);
  rnd_dat = SDL_CreateRenderer(tile_dat, -1, SDL_RENDERER_ACCELERATED);
}

void scnln() {
  if(gt_bt(LCDC, 7)) {
    if(LY < 0x90) {
      if(gt_bt(LCDC, 0)) {
        int dat_area = gt_bt(LCDC, 4);
        int mp_area = gt_bt(LCDC, 3);
        byte scy = SCY;
        byte scx = SCX;
        byte ly = (LY + scy) % 256;
        int tiley = ly / 8;
        int offy = ly % 8;
        int bytey = offy * 2;
        byte pal = BGP;
        for(int x = 0; x < 0xA0; x ++) {
          int lx = (x + scx) % 256;
          int tilex = lx / 8;
          int offx = lx % 8;
          int idx = (tiley * 32) + tilex;
          if(mp_area == 0) idx += 0x9800;
          else idx += 0x9C00;
          idx = r_mem(idx);
          if(dat_area == 0) idx = 255 - idx;
          idx *= 16;
          if(dat_area == 0) idx += 0x8800;
          else idx += 0x8000;
          byte ls = r_mem(idx + bytey);
          byte ms = r_mem(idx + bytey + 1);
          offx = 7 - offx;
          int clr = (gt_bt(ms, offx) << 1) | gt_bt(ls, offx);
          dsp[LY][x] = gt_clr(pal, clr);
        }
      }
    }
    int ly = LY;
    ly ++;
    if(ly >= 154) ly = 0;
    w_mem(0xFF44, ly);
  }
}

void drw_screen() {
  for(int i = 0; i < 0x90; i ++) {
    for(int j = 0; j < 0xA0; j ++) {
      int clr = dsp[i][j];
      if(clr == CLR_WHT) SDL_SetRenderDrawColor(rnd, 0xFF, 0xFF, 0xFF, 0xFF);
      else if(clr == CLR_L_GRY) SDL_SetRenderDrawColor(rnd, 0x7F, 0x7F, 0x7F, 0xFF);
      else if(clr == CLR_D_GRY) SDL_SetRenderDrawColor(rnd, 0x54, 0x54, 0x54, 0xFF);
      else if(clr == CLR_BLK) SDL_SetRenderDrawColor(rnd, 0x00, 0x00, 0x00, 0xFF);
      else printf("INVALID COLOR %d\n", clr);
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

void drw_tile_dat() {
  byte pal = BGP;
  for(int t = 0x8000; t < 0x9800; t += 16) {
    int tile[8][8];
    for(int i = 0; i < 8; i ++) {
      int line = i * 2;
      int ls = r_mem(t + line);
      int ms = r_mem(t + line + 1);
      for(int j = 0; j < 8; j ++) {
        int clr = (gt_bt(ms, 7 - j) << 1) | gt_bt(ls, 7 - j);
        tile[i][j] = gt_clr(pal, clr);
      }
    }
    int curr = (t - 0x8000) / 16;
    int addy = (curr / 16) * 8;
    int addx = (curr % 16) * 8;
    for(int y = 0; y < 8; y ++) {
      for(int x = 0; x < 8; x ++) {
        int clr = tile[y][x];
        if(clr == CLR_WHT) SDL_SetRenderDrawColor(rnd_dat, 0xFF, 0xFF, 0xFF, 0xFF);
        else if(clr == CLR_L_GRY) SDL_SetRenderDrawColor(rnd_dat, 0x7F, 0x7F, 0x7F, 0xFF);
        else if(clr == CLR_D_GRY) SDL_SetRenderDrawColor(rnd_dat, 0x54, 0x54, 0x54, 0xFF);
        else if(clr == CLR_BLK) SDL_SetRenderDrawColor(rnd_dat, 0x00, 0x00, 0x00, 0xFF);

        SDL_Rect rct;
        rct.x = (x + addx) * FCT_DAT;
        rct.y = (y + addy) * FCT_DAT;
        rct.w = FCT_DAT;
        rct.h = FCT_DAT;

        SDL_RenderFillRect(rnd_dat, &rct);
      }
    }
  }
  SDL_RenderPresent(rnd_dat);
}

int rndr() {
  drw_screen();
  drw_tile_dat();
  return 0;
}

#endif
