#ifndef DEF
#define DEF

#include <stdio.h>

#include "var.h"
#include "func.h"

void init_dsp() {
}

void scnln() {
  // TODO Do OAM Search
  if(gt_bt(LCDC_CTRL, 7)) {
    int line = LY;
    if(line < 144) {
      if(gt_bt(LCDC_CTRL, 0)) {
        int dat_area = gt_bt(LCDC_CTRL, 4);
        int mp_area = gt_bt(LCDC_CTRL, 3);
        byte scy = SCY;
        byte scx = SCX;
        byte ly = (line + scy) % 256;
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
          dsp[ly][lx] = clr;
        }
      }
      if(gt_bt(LCDC_CTRL, 2)) {
        // TODO Draw OBJ
      }
    }
    line ++;
    if(line >= 154) line = 0;
    w_mem(0xFF44, line);
  } else {
    for(int i = 0; i < 0x90; i ++) {
      for(int j = 0; j < 0xA0; j ++) {
        dsp[i][j] = ABS_WHT;
      }
    }
  }
}

void rndr() {
  byte pal = BGP;
  for(int i = 0; i < 0x90; i ++) {
    for(int j = 0; j < 0xA0; j ++) {
      int clr = dsp[i][j] * 2;
      clr = (pal >> clr) & 0b11;
      if(clr == ABS_WHT);
      else if(clr == CLR_WHT);
      else if(clr == CLR_L_GRY);
      else if(clr == CLR_D_GRY);
      else if(clr == CLR_BLK);
      else printf("clr\n");
    }
  }
}

#endif
