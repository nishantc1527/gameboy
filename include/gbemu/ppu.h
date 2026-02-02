#pragma once

#include <SDL3/SDL.h>
#include <stdint.h>

#define LCDC r_mem(0xFF40)
#define LCD_STAT r_mem(0xFF41)
#define SCY r_mem(0xFF42)
#define SCX r_mem(0xFF43)
#define LY r_mem(0xFF44)
#define LYC r_mem(0xFF45)
#define DMA r_mem(0xFF46)
#define BGP r_mem(0xFF47)
#define OBP0 r_mem(0xFF48)
#define OBP1 r_mem(0xFF49)
#define WY r_mem(0xFF4A)
#define WX r_mem(0xFF4B)

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

extern uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];

extern uint8_t WIN_CNT;
extern uint16_t scn;
extern uint8_t frame;
extern int in[8];

extern const uint16_t SCANLINE_LEN, SCANLINES;
extern uint8_t headless;

// Initialize window and renderer
void init_ppu(void);

// Handle input
int update_input(void);

// Update registers
void update_lcd(void);

// Perform scanline
void do_scanline(void);
