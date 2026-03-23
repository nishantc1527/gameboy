#pragma once

#include <stdint.h>

#define SCRN_WIDTH 160
#define SCRN_HEIGHT 144

#define PPU_CYCLES_PER_LINE 456
#define PPU_VISIBLE_LINES 144
#define PPU_VBLANK_LINES 10
#define PPU_TOTAL_LINES 154

#define CLR_WHT 0
#define CLR_L_GRY 1
#define CLR_D_GRY 2
#define CLR_BLK 3
#define CLR_EXT 4

struct Bus;

struct Ppu {
  uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint16_t cgb_dsp[SCRN_HEIGHT][SCRN_WIDTH];
  uint16_t scn;
  uint8_t frame_ready;
  uint8_t lcdc_reenable;
  uint8_t stat_irq_line;
  uint8_t off_scn;
  uint8_t cgb_mode;
  uint8_t cgb_compat;
  uint8_t win_cnt;
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

struct Ppu* ppu_init(uint8_t cgb_mode, uint8_t cgb_compat);
uint8_t ppu_read(const struct Ppu* ppu, uint16_t addr);
void ppu_write(struct Ppu* ppu, uint16_t addr, uint8_t val);
void ppu_post_boot(struct Ppu* ppu);
void ppu_tick(struct Ppu* ppu, struct Bus* bus, uint8_t cycles);
uint8_t ppu_frame_ready(const struct Ppu* ppu);
void ppu_begin_frame(struct Ppu* ppu);
uint8_t ppu_blocks_vram(const struct Ppu* ppu);
uint8_t ppu_blocks_oam(const struct Ppu* ppu);
