#pragma once

#include "gbemu/ppu.h"

struct Ppu {
  uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint16_t cgb_dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint16_t line_cycles;
  uint8_t frame_ready;
  uint8_t lcd_turning_on;
  uint8_t stat_irq_line;
  uint8_t off_line_count;
  uint8_t cgb_mode;
  uint8_t cgb_compat;
  uint8_t window_line;
  uint8_t wy_triggered;
  uint8_t lcdc;
  uint8_t stat;
  uint8_t scy, scx;
  uint8_t ly;
  uint8_t lyc;
  uint8_t bgp;
  uint8_t obp0;
  uint8_t obp1;
  uint8_t wy, wx;
  uint8_t bg_pal_idx;
  uint8_t bg_pal_ram[64];
  uint8_t obj_pal_idx;
  uint8_t obj_pal_ram[64];
};

#define PPU_MODE_HBLANK 0
#define PPU_MODE_VBLANK 1
#define PPU_MODE_OAM 2
#define PPU_MODE_TRANSFER 3
#define PPU_MODE_MASK 0x03

#define LCDC_BIT_BG_ENABLE 0
#define LCDC_BIT_OBJ_ENABLE 1
#define LCDC_BIT_OBJ_SIZE 2
#define LCDC_BIT_BG_MAP 3
#define LCDC_BIT_TILE_DATA 4
#define LCDC_BIT_WIN_ENABLE 5
#define LCDC_BIT_WIN_MAP 6
#define LCDC_BIT_LCD_ENABLE 7

#define STAT_LYC_FLAG_BIT 2
#define STAT_INT_HBLANK_BIT 3
#define STAT_INT_VBLANK_BIT 4
#define STAT_INT_OAM_BIT 5
#define STAT_INT_LYC_BIT 6
#define STAT_WRITABLE_MASK 0x78
#define STAT_MODE_PRESERVE 0x07
#define STAT_UNUSED_BIT 0x80

#define PPU_BOOT_LCDC 0x91
#define PPU_BOOT_STAT 0x85
#define PPU_BOOT_BGP 0xFC

#define PAL_IDX_MASK 0x3F
#define PAL_AUTO_INC_BIT 0x80

#define PPU_OAM_END_CYCLE 80
#define PPU_TRANSFER_BASE_END 252

#define TILE_MAP_0 0x9800
#define TILE_MAP_1 0x9C00
#define TILE_DATA_LO 0x8000
#define TILE_DATA_HI 0x8800
#define OBJ_TILE_BASE 0x8000
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define TILE_BYTES 16
#define TILE_MAP_STRIDE 32
#define TILE_COORD_WRAP 256

#define OAM_ENTRY_BYTES 4
#define OAM_MAX_SPRITES 40
#define OAM_LINE_LIMIT 10
#define OBJ_Y_OFFSET 16
#define OBJ_X_OFFSET 8
#define OBJ_TALL_HEIGHT 16

#define OBJ_ATTR_VRAM_BANK_BIT 3
#define OBJ_ATTR_PAL_DMG_BIT 4
#define OBJ_ATTR_FLIP_X_BIT 5
#define OBJ_ATTR_FLIP_Y_BIT 6
#define OBJ_ATTR_PRIORITY_BIT 7
#define OBJ_TALL_TILE_MASK 0xFE

#define OAM_OFF_Y 0
#define OAM_OFF_X 1
#define OAM_OFF_TILE 2
#define OAM_OFF_ATTR 3

struct Bus;

// Writes to pixel buffer
void write_pixel(struct Ppu* ppu, int y, int x, uint8_t clr);
uint8_t palette_color(uint8_t pal, int color_idx);

uint16_t tile_map_base(uint8_t lcdc, int use_window_map);
uint16_t tile_data_addr(uint8_t tile_idx, int dat_area);
int collect_sprites(struct Bus* bus, uint8_t ly, uint8_t sz, uint16_t* obj);

void render_bg_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly);
void render_window_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly);
void render_sprites_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly);

void do_scanline_cgb(struct Ppu* ppu, struct Bus* bus);

void ppu_update_mode(struct Ppu* ppu, struct Bus* bus);
void ppu_render_line(struct Ppu* ppu, struct Bus* bus);
