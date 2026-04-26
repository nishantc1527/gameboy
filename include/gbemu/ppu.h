#pragma once

#include <stdbool.h>
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
struct Ppu;

struct Ppu* ppu_init(bool cgb_mode, bool cgb_compat);
uint8_t ppu_read(const struct Ppu* ppu, uint16_t addr);
void ppu_write(struct Ppu* ppu, uint16_t addr, uint8_t val);
void ppu_tick(struct Ppu* ppu, struct Bus* bus, uint8_t cycles);
bool ppu_frame_ready(const struct Ppu* ppu);
void ppu_begin_frame(struct Ppu* ppu);
bool ppu_blocks_vram(const struct Ppu* ppu);
bool ppu_blocks_oam(const struct Ppu* ppu);
bool ppu_get_cgb_mode(const struct Ppu* ppu);
const uint8_t* ppu_get_dmg_framebuffer(const struct Ppu* ppu);
const uint16_t* ppu_get_cgb_framebuffer(const struct Ppu* ppu);
