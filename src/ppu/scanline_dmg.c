#include <stdint.h>

#include "../bus_private.h"
#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

static void dmg_draw_bg_pixel(struct Ppu* ppu, struct Bus* bus, uint8_t ly,
                              int x, uint8_t tilex, uint8_t tiley, uint8_t offx,
                              int offy, uint16_t map_base, int dat_area,
                              uint8_t pal) {
  uint16_t map_offs =
      (uint16_t)((uint16_t)tiley * (uint16_t)TILE_MAP_STRIDE) + (uint16_t)tilex;
  uint8_t tile_idx = mmu_read_vram(bus->mmu, (uint16_t)(map_base + map_offs));
  uint16_t idx = tile_data_addr(tile_idx, dat_area);
  int ty = offy << 1;
  uint8_t ls = mmu_read_vram(bus->mmu, (uint16_t)(idx + (uint16_t)ty));
  uint8_t ms = mmu_read_vram(bus->mmu, (uint16_t)(idx + (uint16_t)ty + 1));
  offx = TILE_WIDTH - 1 - offx;
  int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
  write_pixel(ppu, ly, x, palette_color(pal, clr));
}

void render_bg_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
  int dat_area = get_bit(ppu->lcdc, LCDC_BIT_TILE_DATA);
  uint16_t map_base = tile_map_base(ppu->lcdc, 0);
  uint8_t pal = ppu->bgp;
  uint8_t by = (ly + ppu->scy) % TILE_COORD_WRAP;
  uint8_t tiley = by / (uint8_t)TILE_HEIGHT;
  int offy = by % TILE_HEIGHT;
  for (int x = 0; x < SCRN_WIDTH; x++) {
    uint8_t bx = (uint8_t)((x + ppu->scx) % TILE_COORD_WRAP);
    dmg_draw_bg_pixel(ppu, bus, ly, x, bx / TILE_WIDTH, tiley, bx % TILE_WIDTH,
                      offy, map_base, dat_area, pal);
  }
}

void render_window_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
  uint8_t wx = ppu->wx;
  if (wx >= SCRN_WIDTH + 7 || !ppu->wy_triggered) return;
  wx = wx - 7;
  int dat_area = get_bit(ppu->lcdc, LCDC_BIT_TILE_DATA);
  uint16_t map_base = tile_map_base(ppu->lcdc, 1);
  uint8_t pal = ppu->bgp;
  uint8_t wy = ppu->window_line;
  uint8_t tiley = wy / TILE_HEIGHT;
  int offy = wy % TILE_HEIGHT;
  for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
    uint8_t _wx = x - wx;
    dmg_draw_bg_pixel(ppu, bus, ly, x, _wx / TILE_WIDTH, tiley,
                      _wx % TILE_WIDTH, offy, map_base, dat_area, pal);
  }
  ppu->window_line++;
}

void render_sprites_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
  uint8_t sz = get_bit(ppu->lcdc, LCDC_BIT_OBJ_SIZE);
  uint16_t obj[OAM_LINE_LIMIT] = {0};
  int cnt = collect_sprites(bus, ly, sz, obj);
  uint16_t maxx = TILE_COORD_WRAP;
  uint16_t maxm = 0xFFFF;
  while (cnt--) {
    int midx = -1;
    for (int i = 0; i < OAM_LINE_LIMIT; i++)
      if (obj[i]) {
        uint8_t x =
            mmu_read_oam(bus->mmu, (uint16_t)(obj[i] - OAM_START + OAM_OFF_X));
        if (x < maxx || (x == maxx && obj[i] < maxm)) {
          if (midx == -1)
            midx = i;
          else {
            uint8_t prev = mmu_read_oam(
                bus->mmu, (uint16_t)(obj[midx] - OAM_START + OAM_OFF_X));
            if (x > prev) midx = i;
            if (x == prev && obj[i] > obj[midx]) midx = i;
          }
        }
      }
    if (midx == -1) break;
    uint16_t mem_loc = obj[midx];
    obj[midx] = 0;
    maxx = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    maxm = mem_loc;
    uint8_t y =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
    int x = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    uint16_t idx =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
    uint8_t flg =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
    y -= OBJ_Y_OFFSET;
    x -= OBJ_X_OFFSET;
    if (sz) idx &= OBJ_TALL_TILE_MASK;
    idx *= TILE_BYTES;
    idx += OBJ_TILE_BASE;
    uint8_t flipx = get_bit(flg, OBJ_ATTR_FLIP_X_BIT);
    uint8_t flipy = get_bit(flg, OBJ_ATTR_FLIP_Y_BIT);
    uint8_t line = ly - y;
    if (sz) {
      if (flipy) line = OBJ_TALL_HEIGHT - 1 - line;
    } else {
      if (flipy) line = TILE_HEIGHT - 1 - line;
    }
    line = (uint8_t)(line << 1);
    uint8_t ls = mmu_read_vram_bank0(bus->mmu, idx + line + 0);
    uint8_t ms =
        mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + (uint16_t)line + 1));
    uint8_t pal = get_bit(flg, OBJ_ATTR_PAL_DMG_BIT) ? ppu->obp1 : ppu->obp0;
    for (int x0 = x; x0 < x + TILE_WIDTH; x0++) {
      if (x0 < 0) continue;
      uint8_t posx = (uint8_t)(TILE_WIDTH - 1) - (uint8_t)(x0 - x);
      if (flipx) posx = TILE_WIDTH - 1 - posx;
      uint8_t clr = (uint8_t)(get_bit(ms, posx) << 1) | get_bit(ls, posx);
      if (get_bit(flg, OBJ_ATTR_PRIORITY_BIT)) {
        if (ppu->dsp[ly][x0] == palette_color(ppu->bgp, 0))
          write_pixel(ppu, ly, x0, palette_color(pal, clr));
      } else if (clr != 0)
        write_pixel(ppu, ly, x0, palette_color(pal, clr));
    }
  }
}
