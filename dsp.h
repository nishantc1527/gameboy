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

  win = SDL_CreateWindow("GAMEBOY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160, 144, 0);
  rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
}

void scnln() {
  if(gt_bt(LCDC_CTRL, 7)) {
    // TODO Do OAM Search
    if(LY < 0x90) {
      if(gt_bt(LCDC_CTRL, 0)) {
        int dat_area = gt_bt(LCDC_CTRL, 4);
        int mp_area = gt_bt(LCDC_CTRL, 3);
        byte scy = SCY;
        byte scx = SCX;
        byte ly = (LY + scy) % 256;
        int tiley = ly / 8;
        int offy = ly % 8;
        int bytey = offy / 2;
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
          // printf("LX: %x, LY: %x\n", lx, ly);
          dsp[LY][lx] = clr;
        }
      }
      if(gt_bt(LCDC_CTRL, 2)) {
        // TODO Draw OBJ
      }
    }
    int ly = LY;
    ly ++;
    if(ly >= 154) ly = 0;
    w_mem(0xFF44, ly);
  } else {
    for(int i = 0; i < 0x90; i ++) {
      for(int j = 0; j < 0xA0; j ++) {
        dsp[i][j] = ABS_WHT;
      }
    }
  }
}

int rndr() {
  while(SDL_PollEvent(&evt)) {
    if(evt.type == SDL_QUIT) return 1;
  }
  byte pal = BGP;
  for(int i = 0; i < 0x90; i ++) {
    for(int j = 0; j < 0xA0; j ++) {
      int clr = dsp[i][j] * 2;
      clr = (pal >> clr) & 0b11;
      if(clr == ABS_WHT) SDL_SetRenderDrawColor(rnd, 0xFF, 0xFF, 0xFF, 0xFF);
      else if(clr == CLR_WHT) SDL_SetRenderDrawColor(rnd, 0xBE, 0xBE, 0xBE, 0xFF);
      else if(clr == CLR_L_GRY) SDL_SetRenderDrawColor(rnd, 0x7F, 0x7F, 0x7F, 0xFF);
      else if(clr == CLR_D_GRY) SDL_SetRenderDrawColor(rnd, 0x54, 0x54, 0x54, 0xFF);
      else if(clr == CLR_BLK) SDL_SetRenderDrawColor(rnd, 0x00, 0x00, 0x00, 0xFF);
      else printf("clr\n");
      SDL_Rect rct;
      rct.x = j;
      rct.y = i;
      rct.w = 1;
      rct.h = 1;
      SDL_RenderFillRect(rnd, &rct);
    }
  }
  SDL_RenderPresent(rnd);
  return 0;
}

#endif
