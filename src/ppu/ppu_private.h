#pragma once

#include "gbemu/ppu.h"

struct Bus;

// Writes to pixel buffer
void w_pxl(struct Ppu* ppu, int y, int x, uint8_t clr);

uint8_t gt_clr(uint8_t pal, int val);

void update_lcd(struct Ppu* ppu, struct Bus* bus);
void do_scanline(struct Ppu* ppu, struct Bus* bus);
