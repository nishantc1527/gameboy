#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

uint8_t palette_color(uint8_t pal, int color_idx) {
  return (pal >> (color_idx << 1)) & PPU_MODE_MASK;
}

void write_pixel(struct Ppu* ppu, int y, int x, uint8_t clr) {
  ppu->dsp[y][x] = clr;
}

uint16_t tile_map_base(uint8_t lcdc, int use_window_map) {
  int mp_area = use_window_map ? get_bit(lcdc, LCDC_BIT_WIN_MAP)
                               : get_bit(lcdc, LCDC_BIT_BG_MAP);
  return mp_area ? TILE_MAP_1 : TILE_MAP_0;
}

uint16_t tile_data_addr(uint8_t tile_idx, int dat_area) {
  uint16_t idx = tile_idx;
  if (dat_area == 0) idx = (uint16_t)((int8_t)idx + 128);
  idx = (uint16_t)(idx * TILE_BYTES);
  idx = (uint16_t)(idx + (uint16_t)(dat_area ? TILE_DATA_LO : TILE_DATA_HI));
  return idx;
}

int collect_sprites(struct Bus* bus, uint8_t ly, uint8_t sz, uint16_t* obj) {
  int cnt = 0;
  for (uint16_t mem_loc = OAM_START; mem_loc <= OAM_END && cnt < OAM_LINE_LIMIT;
       mem_loc += OAM_ENTRY_BYTES) {
    int y = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START));
    y -= OBJ_Y_OFFSET;
    if (ly < y) continue;
    if (sz) {
      if (ly >= y + OBJ_TALL_HEIGHT) continue;
    } else if (ly >= y + TILE_HEIGHT)
      continue;
    obj[cnt++] = mem_loc;
  }
  return cnt;
}

static void advance_line(struct Ppu* ppu) {
  ppu->ly++;
  if (ppu->ly >= PPU_TOTAL_LINES) {
    ppu->ly = 0;
    ppu->window_line = 0;
    ppu->wy_triggered = 0;
    ppu->frame_ready = true;
  }
}

void ppu_render_line(struct Ppu* ppu, struct Bus* bus) {
  if (get_bit(ppu->lcdc, LCDC_BIT_LCD_ENABLE)) {
    if (ppu->ly < PPU_VISIBLE_LINES) {
      if (ppu->cgb_mode) {
        do_scanline_cgb(ppu, bus);
        advance_line(ppu);
        return;
      }
      if (get_bit(ppu->lcdc, LCDC_BIT_BG_ENABLE)) {
        render_bg_dmg(ppu, bus, ppu->ly);
        if (get_bit(ppu->lcdc, LCDC_BIT_WIN_ENABLE))
          render_window_dmg(ppu, bus, ppu->ly);
      } else {
        for (int x = 0; x < SCRN_WIDTH; x++) {
          write_pixel(ppu, ppu->ly, x, CLR_WHT);
        }
      }
      if (get_bit(ppu->lcdc, LCDC_BIT_OBJ_ENABLE))
        render_sprites_dmg(ppu, bus, ppu->ly);
    }
    advance_line(ppu);
  } else {
    ppu->ly = 0;
    ppu->window_line = 0;
    ppu->wy_triggered = 0;
    ppu->off_line_count++;
    if (ppu->off_line_count >= PPU_TOTAL_LINES) {
      ppu->off_line_count = 0;
      ppu->frame_ready = true;
    }
  }
}
