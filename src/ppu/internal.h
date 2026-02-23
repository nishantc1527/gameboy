#pragma once

#include <stdint.h>

#include "gbemu/ppu.h"

// Writes to pixel buffer
void w_pxl(struct Ppu* ppu, int y, int x, uint8_t clr);

uint8_t gt_clr(uint8_t pal, int val);
