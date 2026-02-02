#pragma once

#include <stdint.h>

// Writes to pixel buffer
void w_pxl(int y, int x, int clr);

int gt_clr(uint8_t pal, int val);
