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
    if(gt_bt(LCDC_CTRL, 0)) {
      int dat_area = gt_bt(LCDC_CTRL, 4);
      int mp_area = gt_bt(LCDC_CTRL, 3);
      byte scy = SCY;
      byte scx = SCX;
      byte ly = LY;
    }
    if(gt_bt(LCDC_CTRL, 2)) {
      // TODO Draw OBJ
    }
  } else {
    for(int i = 0; i < 0xA0; i ++) {
      for(int j = 0; j < 0x90; j ++) {
        dsp[i][j] = CLR_WHT;
      }
    }
  }
}

void rndr() {
  for(int i = 0; i < 0xA0; i ++) {
    for(int j = 0; j < 0x90; j ++) {
      
    }
  }
}

#endif
