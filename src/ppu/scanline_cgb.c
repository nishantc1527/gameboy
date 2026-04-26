#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

typedef struct {
  uint8_t color;
  uint8_t pal_num;
  uint8_t bg_prio;
} CgbBgPixel;

typedef struct {
  CgbBgPixel buf[FIFO_CAPACITY];
  int head, size;
} CgbFifo;

static void cgb_fifo_clear(CgbFifo* f) {
  f->head = 0;
  f->size = 0;
}

static void cgb_fifo_push(CgbFifo* f, uint8_t lo, uint8_t hi, uint8_t pal_num,
                          uint8_t bg_prio, uint8_t xflip) {
  for (int i = 0; i < 8; i++) {
    int bit = xflip ? i : (7 - i);
    f->buf[(f->head + f->size++) & (FIFO_CAPACITY - 1)] = (CgbBgPixel){
        .color = (uint8_t)(((hi >> bit) & 1) << 1 | ((lo >> bit) & 1)),
        .pal_num = pal_num,
        .bg_prio = bg_prio,
    };
  }
}

static CgbBgPixel cgb_fifo_pop(CgbFifo* f) {
  CgbBgPixel px = f->buf[f->head & (FIFO_CAPACITY - 1)];
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
  uint8_t tile_idx;
  uint8_t pal_num;
  uint8_t vram_bank;
  uint8_t xflip;
  uint8_t yflip;
  uint8_t bg_prio;
  uint8_t lo, hi;
} CgbFetcher;

static void cgb_fetcher_init(CgbFetcher* f, int tx, int tiley, int offy,
                             uint16_t map, int dat_area) {
  f->step = 0;
  f->tx = tx & (TILE_MAP_STRIDE - 1);
  f->tiley = tiley;
  f->offy = offy;
  f->map = map;
  f->dat_area = dat_area;
  f->tile_idx = f->pal_num = f->vram_bank = 0;
  f->xflip = f->yflip = f->bg_prio = f->lo = f->hi = 0;
}

static void cgb_fetcher_tick(CgbFetcher* f, CgbFifo* fifo, struct Bus* bus) {
  switch (f->step) {
    case 0:
      f->step = 1;
      break;
    case 1: {
      uint16_t offs =
          (uint16_t)((uint16_t)f->tiley * TILE_MAP_STRIDE + (uint16_t)f->tx);
      uint16_t vaddr = (uint16_t)(f->map + offs);
      f->tile_idx = mmu_read_vram_bank0(bus->mmu, vaddr);
      uint8_t attr = mmu_read_vram_bank1(bus->mmu, vaddr);
      f->pal_num = attr & 7u;
      f->vram_bank = (attr >> OBJ_ATTR_VRAM_BANK_BIT) & 1u;
      f->xflip = (attr >> OBJ_ATTR_FLIP_X_BIT) & 1u;
      f->yflip = (attr >> OBJ_ATTR_FLIP_Y_BIT) & 1u;
      f->bg_prio = (attr >> OBJ_ATTR_PRIORITY_BIT) & 1u;
      f->step = 2;
      break;
    }
    case 2:
      f->step = 3;
      break;
    case 3: {
      int row = f->yflip ? (TILE_HEIGHT - 1 - f->offy) : f->offy;
      uint16_t addr = tile_data_addr(f->tile_idx, f->dat_area);
      uint16_t off = (uint16_t)(addr + (uint16_t)(row << 1));
      f->lo = f->vram_bank ? mmu_read_vram_bank1(bus->mmu, off)
                           : mmu_read_vram_bank0(bus->mmu, off);
      f->step = 4;
      break;
    }
    case 4:
      f->step = 5;
      break;
    case 5: {
      int row = f->yflip ? (TILE_HEIGHT - 1 - f->offy) : f->offy;
      uint16_t addr = tile_data_addr(f->tile_idx, f->dat_area);
      uint16_t off = (uint16_t)(addr + (uint16_t)(row << 1) + 1u);
      f->hi = f->vram_bank ? mmu_read_vram_bank1(bus->mmu, off)
                           : mmu_read_vram_bank0(bus->mmu, off);
      f->step = 6;
      break;
    }
    case 6:
      if (fifo->size == 0) {
        cgb_fifo_push(fifo, f->lo, f->hi, f->pal_num, f->bg_prio, f->xflip);
        f->tx = (f->tx + 1) & (TILE_MAP_STRIDE - 1);
        f->step = 0;
      }
      break;
  }
}

static void render_sprites_cgb(struct Ppu* ppu, struct Bus* bus, uint8_t ly,
                               const uint8_t* bg_color_idx,
                               const uint8_t* bg_prio_bit) {
  uint8_t lcdc = ppu->lcdc;
  uint8_t sz = get_bit(lcdc, LCDC_BIT_OBJ_SIZE);
  uint16_t obj[OAM_LINE_LIMIT] = {0};
  int cnt = collect_sprites(bus, ly, sz, obj);
  for (int i = cnt - 1; i >= 0; i--) {
    uint16_t mem_loc = obj[i];
    uint8_t y =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
    int x = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    uint16_t tile_idx =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
    uint8_t flg =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
    y -= OBJ_Y_OFFSET;
    x -= OBJ_X_OFFSET;
    if (sz) tile_idx &= OBJ_TALL_TILE_MASK;
    uint8_t vram_bank = (flg >> OBJ_ATTR_VRAM_BANK_BIT) & 1;
    uint8_t pal_num =
        ppu->cgb_compat ? ((flg >> OBJ_ATTR_PAL_DMG_BIT) & 1) : (flg & 7);
    uint8_t flipx = get_bit(flg, OBJ_ATTR_FLIP_X_BIT);
    uint8_t flipy = get_bit(flg, OBJ_ATTR_FLIP_Y_BIT);
    uint8_t obj_prio = get_bit(flg, OBJ_ATTR_PRIORITY_BIT);
    uint16_t idx = (uint16_t)(tile_idx * TILE_BYTES + OBJ_TILE_BASE);
    uint8_t line = (uint8_t)(ly - y);
    if (sz) {
      if (flipy) line = OBJ_TALL_HEIGHT - 1 - line;
    } else {
      if (flipy) line = TILE_HEIGHT - 1 - line;
    }
    line = (uint8_t)(line << 1);
    uint8_t ls, ms;
    if (vram_bank == 0) {
      ls = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + line));
      ms = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + line + 1));
    } else {
      ls = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + line));
      ms = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + line + 1));
    }
    for (int x0 = x; x0 < x + TILE_WIDTH; x0++) {
      if (x0 < 0 || x0 >= SCRN_WIDTH) continue;
      uint8_t posx = (uint8_t)(x0 - x);
      if (!flipx) posx = TILE_WIDTH - 1 - posx;
      uint8_t clr = (uint8_t)((get_bit(ms, posx) << 1) | get_bit(ls, posx));
      if (clr == 0) continue;
      int bg_wins = get_bit(lcdc, LCDC_BIT_BG_ENABLE) &&
                    bg_color_idx[x0] != 0 && (bg_prio_bit[x0] || obj_prio);
      if (!bg_wins) {
        uint8_t lo = ppu->obj_pal_ram[pal_num * 8 + clr * 2];
        uint8_t hi = ppu->obj_pal_ram[pal_num * 8 + clr * 2 + 1];
        ppu->cgb_dsp[ly][x0] = (uint16_t)(lo | ((uint16_t)hi << 8));
      }
    }
  }
}

void render_line_cgb(struct Ppu* ppu, struct Bus* bus) {
  uint8_t ly = ppu->ly;
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
  CgbFetcher fetcher;
  cgb_fetcher_init(&fetcher, init_tx, bg_tiley, bg_offy, bg_map, dat_area);
  CgbFifo fifo;
  cgb_fifo_clear(&fifo);
  int discard = scx & 7;
  int px_out = 0;
  uint8_t bg_color_idx[SCRN_WIDTH];
  uint8_t bg_prio_bit[SCRN_WIDTH];
  while (px_out < SCRN_WIDTH) {
    if (win_enabled && !win_started && discard == 0 && px_out == wx) {
      cgb_fifo_clear(&fifo);
      cgb_fetcher_init(&fetcher, 0, win_tiley, win_offy, win_map, dat_area);
      win_started = 1;
    }
    cgb_fetcher_tick(&fetcher, &fifo, bus);
    if (fifo.size > 0) {
      CgbBgPixel px = cgb_fifo_pop(&fifo);
      if (discard > 0) {
        discard--;
      } else {
        bg_color_idx[px_out] = px.color;
        bg_prio_bit[px_out] = px.bg_prio;
        uint8_t lo = ppu->bg_pal_ram[px.pal_num * 8 + px.color * 2];
        uint8_t hi = ppu->bg_pal_ram[px.pal_num * 8 + px.color * 2 + 1];
        ppu->cgb_dsp[ly][px_out] = (uint16_t)(lo | ((uint16_t)hi << 8));
        px_out++;
      }
    }
  }
  if (win_started) ppu->window_line++;
  if (get_bit(lcdc, LCDC_BIT_OBJ_ENABLE))
    render_sprites_cgb(ppu, bus, ly, bg_color_idx, bg_prio_bit);
}
