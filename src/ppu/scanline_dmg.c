#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

typedef struct {
  uint8_t color;
} BgPixel;

typedef struct {
  BgPixel buf[FIFO_CAPACITY];
  int head, size;
} BgFifo;

static void fifo_clear(BgFifo* f) {
  f->head = 0;
  f->size = 0;
}

static void fifo_push(BgFifo* f, uint8_t lo, uint8_t hi) {
  for (int b = 7; b >= 0; b--)
    f->buf[(f->head + f->size++) & (FIFO_CAPACITY - 1)].color =
        (uint8_t)(((hi >> b) & 1) << 1 | ((lo >> b) & 1));
}

static BgPixel fifo_pop(BgFifo* f) {
  BgPixel px = f->buf[f->head & (FIFO_CAPACITY - 1)];
  f->head = (f->head + 1) & (FIFO_CAPACITY - 1);
  f->size--;
  return px;
}

typedef struct {
  int step;
  int tx;
  int tiley;
  int offy;
  uint16_t map;
  int dat_area;
  uint8_t tile_idx, lo, hi;
} Fetcher;

static void fetcher_init(Fetcher* f, int tx, int tiley, int offy, uint16_t map,
                         int dat_area) {
  f->step = 0;
  f->tx = tx & (TILE_MAP_STRIDE - 1);
  f->tiley = tiley;
  f->offy = offy;
  f->map = map;
  f->dat_area = dat_area;
  f->tile_idx = f->lo = f->hi = 0;
}

static void fetcher_tick(Fetcher* f, BgFifo* fifo, struct Bus* bus) {
  switch (f->step) {
    case 0:
      f->step = 1;
      break;
    case 1: {
      uint16_t offs =
          (uint16_t)((uint16_t)f->tiley * TILE_MAP_STRIDE + (uint16_t)f->tx);
      f->tile_idx = mmu_read_vram(bus->mmu, (uint16_t)(f->map + offs));
      f->step = 2;
      break;
    }
    case 2:
      f->step = 3;
      break;
    case 3: {
      uint16_t addr = tile_data_addr(f->tile_idx, f->dat_area);
      f->lo =
          mmu_read_vram(bus->mmu, (uint16_t)(addr + (uint16_t)(f->offy << 1)));
      f->step = 4;
      break;
    }
    case 4:
      f->step = 5;
      break;
    case 5: {
      uint16_t addr = tile_data_addr(f->tile_idx, f->dat_area);
      f->hi = mmu_read_vram(bus->mmu,
                            (uint16_t)(addr + (uint16_t)(f->offy << 1) + 1u));
      f->step = 6;
      break;
    }
    case 6:
      if (fifo->size == 0) {
        fifo_push(fifo, f->lo, f->hi);
        f->tx = (f->tx + 1) & (TILE_MAP_STRIDE - 1);
        f->step = 0;
      }
      break;
  }
}

static void render_bg_window_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
  uint8_t lcdc = ppu->lcdc;
  uint8_t scx = ppu->render_scx;
  int dat_area = get_bit(lcdc, LCDC_BIT_TILE_DATA);
  uint8_t bg_y = (uint8_t)((ly + ppu->scy) % TILE_COORD_WRAP);
  int bg_tiley = bg_y / TILE_HEIGHT;
  int bg_offy = bg_y % TILE_HEIGHT;
  uint16_t bg_map = tile_map_base(lcdc, 0);
  int init_tx = (scx >> 3) & (TILE_MAP_STRIDE - 1);
  int win_enabled = get_bit(lcdc, LCDC_BIT_WIN_ENABLE) && ppu->wy_triggered;
  int wx = 0;
  int win_tiley = 0, win_offy = 0;
  uint16_t win_map = 0;
  int win_started = 0;
  if (win_enabled) {
    if (ppu->wx < 7 || ppu->wx >= SCRN_WIDTH + 7) {
      win_enabled = 0;
    } else {
      wx = (int)ppu->wx - 7;
      win_tiley = ppu->window_line / TILE_HEIGHT;
      win_offy = ppu->window_line % TILE_HEIGHT;
      win_map = tile_map_base(lcdc, 1);
    }
  }
  Fetcher fetcher;
  fetcher_init(&fetcher, init_tx, bg_tiley, bg_offy, bg_map, dat_area);
  BgFifo fifo;
  fifo_clear(&fifo);
  int discard = scx & 7;
  int px_out = 0;
  while (px_out < SCRN_WIDTH) {
    if (win_enabled && !win_started && discard == 0 && px_out == wx) {
      fifo_clear(&fifo);
      fetcher_init(&fetcher, 0, win_tiley, win_offy, win_map, dat_area);
      win_started = 1;
    }
    fetcher_tick(&fetcher, &fifo, bus);
    if (fifo.size > 0) {
      BgPixel px = fifo_pop(&fifo);
      if (discard > 0) {
        discard--;
      } else {
        write_pixel(ppu, ly, px_out, palette_color(ppu->bgp, px.color));
        px_out++;
      }
    }
  }
  if (win_started) ppu->window_line++;
}

static void render_sprites_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
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

void render_line_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t ly) {
  if (get_bit(ppu->lcdc, LCDC_BIT_BG_ENABLE)) {
    render_bg_window_dmg(ppu, bus, ly);
  } else {
    uint8_t blank = palette_color(ppu->bgp, 0);
    for (int x = 0; x < SCRN_WIDTH; x++) write_pixel(ppu, ly, x, blank);
  }
  if (get_bit(ppu->lcdc, LCDC_BIT_OBJ_ENABLE)) render_sprites_dmg(ppu, bus, ly);
}
