#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

uint8_t palette_color(uint8_t pal, int color_idx) {
  return (uint8_t)(((unsigned)pal >> ((unsigned)color_idx << 1U)) & 0x03U);
}

void write_pixel(struct Ppu* ppu, int row, int col, uint8_t clr) {
  ppu->dsp[row][col] = clr;
}

uint16_t tile_map_base(uint8_t lcdc, int use_window_map) {
  int mp_area = use_window_map ? get_bit(lcdc, LCDC_BIT_WIN_MAP)
                               : get_bit(lcdc, LCDC_BIT_BG_MAP);
  return mp_area ? TILE_MAP_1 : TILE_MAP_0;
}

uint16_t tile_data_addr(uint8_t tile_idx, int dat_area) {
  uint16_t idx = tile_idx;
  if (dat_area == 0) {
    idx = (uint16_t)((int8_t)idx + 128);
  }
  idx = (uint16_t)(idx * TILE_BYTES);
  idx = (uint16_t)(idx + (uint16_t)(dat_area ? TILE_DATA_LO : TILE_DATA_HI));
  return idx;
}

int collect_sprites(struct Bus* bus, uint8_t line_y, uint8_t obj_size,
                    uint16_t* obj) {
  int cnt = 0;
  for (uint16_t mem_loc = OAM_START; mem_loc <= OAM_END && cnt < OAM_LINE_LIMIT;
       mem_loc += OAM_ENTRY_BYTES) {
    int spr_y = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START));
    spr_y -= OBJ_Y_OFFSET;
    if (line_y < spr_y) {
      continue;
    }
    if (obj_size) {
      if (line_y >= spr_y + OBJ_TALL_HEIGHT) {
        continue;
      }
    } else if (line_y >= spr_y + TILE_HEIGHT) {
      {
        continue;
      }
    }
    obj[cnt++] = mem_loc;
  }
  return cnt;
}

static void advance_line(struct Ppu* ppu) {
  ppu->ly++;
  if (ppu->ly >= PPU_TOTAL_LINES) {
    ppu->ly = 0;
    ppu->window_line = 0;
    ppu->wy_triggered = false;
    ppu->frame_ready = true;
  }
}

void ppu_render_line(struct Ppu* ppu, struct Bus* bus) {
  if (get_bit(ppu->lcdc, LCDC_BIT_LCD_ENABLE)) {
    if (ppu->ly < PPU_VISIBLE_LINES) {
      if (ppu->cgb_mode) {
        render_line_cgb(ppu, bus);
        advance_line(ppu);
        return;
      }
      render_line_dmg(ppu, bus, ppu->ly);
    }
    advance_line(ppu);
  } else {
    ppu->ly = 0;
    ppu->window_line = 0;
    ppu->wy_triggered = false;
    ppu->off_line_count++;
    if (ppu->off_line_count >= PPU_TOTAL_LINES) {
      ppu->off_line_count = 0;
      ppu->frame_ready = true;
    }
  }
}
