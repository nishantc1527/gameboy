#pragma once

#include <stdint.h>

#define SCRN_WIDTH 0xA0
#define SCRN_HEIGHT 0x90

#define CLR_WHT 0
#define CLR_L_GRY 1
#define CLR_D_GRY 2
#define CLR_BLK 3
#define CLR_EXT 4

struct Bus;

extern const uint16_t SCANLINE_LEN, SCANLINES;

struct Ppu {
  uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint16_t cgb_dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint8_t WIN_CNT;
  uint16_t scn;
  uint8_t frame;
  uint8_t off_scn;
  int in[8];
  uint8_t cgb_mode;
  uint8_t cgb_compat;
};

struct Ppu* init_ppu(void);

void update_lcd(struct Ppu* ppu, struct Bus* bus);

void do_scanline(struct Ppu* ppu, struct Bus* bus);
