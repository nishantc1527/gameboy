#pragma once

#include <stdint.h>

#include "gbemu/mmu.h"

#define BTN_A 0
#define BTN_B 1
#define BTN_START 2
#define BTN_SELECT 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_LEFT 6
#define BTN_RIGHT 7

#define SCRN_WIDTH 0xA0
#define SCRN_HEIGHT 0x90

#define CLR_WHT 0
#define CLR_L_GRY 1
#define CLR_D_GRY 2
#define CLR_BLK 3
#define CLR_EXT 4

extern const uint16_t SCANLINE_LEN, SCANLINES;

struct PPU {
  uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint8_t WIN_CNT;
  uint16_t scn;
  uint8_t frame;
  int in[8];
};

struct PPU* init_ppu(void);

// Handle input
int update_input(struct PPU* ppu, Mmu* mmu);

// Update registers
void update_lcd(struct PPU* ppu, Mmu* mmu);

// Perform scanline
void do_scanline(struct PPU* ppu, Mmu* mmu);
