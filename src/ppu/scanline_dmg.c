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

static void fifo_clear(BgFifo* que) {
  que->head = 0;
  que->size = 0;
}

static void fifo_push(BgFifo* que, uint8_t lo_byte, uint8_t hi_byte) {
  for (int bit = 7; bit >= 0; bit--) {
    que->buf[(unsigned)(que->head + que->size++) &
             (unsigned)(FIFO_CAPACITY - 1)]
        .color = (uint8_t)((((unsigned)hi_byte >> (unsigned)bit) & 1U) << 1U |
                           (((unsigned)lo_byte >> (unsigned)bit) & 1U));
  }
}

static BgPixel fifo_pop(BgFifo* que) {
  BgPixel pixel = que->buf[(unsigned)que->head & (unsigned)(FIFO_CAPACITY - 1)];
  que->head = (int)(((unsigned)que->head + 1U) & (unsigned)(FIFO_CAPACITY - 1));
  que->size--;
  return pixel;
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

static void fetcher_init(Fetcher* ftc, int tile_x, int tiley, int offy,
                         uint16_t map, int dat_area) {
  ftc->step = 0;
  ftc->tx = (int)((unsigned)tile_x & (unsigned)(TILE_MAP_STRIDE - 1));
  ftc->tiley = tiley;
  ftc->offy = offy;
  ftc->map = map;
  ftc->dat_area = dat_area;
  ftc->tile_idx = ftc->lo = ftc->hi = 0;
}

static void fetcher_tick(Fetcher* ftc, BgFifo* que, struct Bus* bus) {
  switch (ftc->step) {
    case 0:
      ftc->step = 1;
      break;
    case 1: {
      uint16_t offs = (uint16_t)(((uint16_t)ftc->tiley * TILE_MAP_STRIDE) +
                                 (uint16_t)ftc->tx);
      ftc->tile_idx = mmu_read_vram(bus->mmu, (uint16_t)(ftc->map + offs));
      ftc->step = 2;
      break;
    }
    case 2:
      ftc->step = 3;
      break;
    case 3: {
      uint16_t addr = tile_data_addr(ftc->tile_idx, ftc->dat_area);
      ftc->lo = mmu_read_vram(bus->mmu,
                              (uint16_t)(addr + (uint16_t)(ftc->offy << 1)));
      ftc->step = 4;
      break;
    }
    case 4:
      ftc->step = 5;
      break;
    case 5: {
      uint16_t addr = tile_data_addr(ftc->tile_idx, ftc->dat_area);
      ftc->hi = mmu_read_vram(
          bus->mmu, (uint16_t)(addr + (uint16_t)(ftc->offy << 1) + 1U));
      ftc->step = 6;
      break;
    }
    case 6:
      if (que->size == 0) {
        fifo_push(que, ftc->lo, ftc->hi);
        ftc->tx =
            (int)(((unsigned)ftc->tx + 1U) & (unsigned)(TILE_MAP_STRIDE - 1));
        ftc->step = 0;
      }
      break;
    default:
      break;
  }
}

static void render_bg_window_dmg(struct Ppu* ppu, struct Bus* bus,
                                 uint8_t line_y) {
  uint8_t lcdc = ppu->lcdc;
  uint8_t scx = ppu->render_scx;
  int dat_area = get_bit(lcdc, LCDC_BIT_TILE_DATA);
  uint8_t bg_y = (uint8_t)((line_y + ppu->scy) % TILE_COORD_WRAP);
  int bg_tiley = bg_y / TILE_HEIGHT;
  int bg_offy = bg_y % TILE_HEIGHT;
  uint16_t bg_map = tile_map_base(lcdc, 0);
  int init_tx = (int)(((unsigned)scx >> 3U) & (unsigned)(TILE_MAP_STRIDE - 1));
  int win_enabled = get_bit(lcdc, LCDC_BIT_WIN_ENABLE) && ppu->wy_triggered;
  int win_x = 0;
  int win_tiley = 0;
  int win_offy = 0;
  uint16_t win_map = 0;
  int win_started = 0;
  if (win_enabled) {
    if (ppu->wx < 7 || ppu->wx >= SCRN_WIDTH + 7) {
      win_enabled = 0;
    } else {
      win_x = (int)ppu->wx - 7;
      win_tiley = ppu->window_line / TILE_HEIGHT;
      win_offy = ppu->window_line % TILE_HEIGHT;
      win_map = tile_map_base(lcdc, 1);
    }
  }
  Fetcher fetcher;
  fetcher_init(&fetcher, init_tx, bg_tiley, bg_offy, bg_map, dat_area);
  BgFifo fifo;
  fifo_clear(&fifo);
  int discard = (int)((unsigned)scx & 7U);
  int px_out = 0;
  while (px_out < SCRN_WIDTH) {
    if (win_enabled && !win_started && discard == 0 && px_out == win_x) {
      fifo_clear(&fifo);
      fetcher_init(&fetcher, 0, win_tiley, win_offy, win_map, dat_area);
      win_started = 1;
    }
    fetcher_tick(&fetcher, &fifo, bus);
    if (fifo.size > 0) {
      BgPixel pixel = fifo_pop(&fifo);
      if (discard > 0) {
        discard--;
      } else {
        write_pixel(ppu, line_y, px_out, palette_color(ppu->bgp, pixel.color));
        px_out++;
      }
    }
  }
  if (win_started) {
    ppu->window_line++;
  }
}

static void render_sprites_dmg(struct Ppu* ppu, struct Bus* bus,
                               uint8_t line_y) {
  uint8_t obj_size = get_bit(ppu->lcdc, LCDC_BIT_OBJ_SIZE);
  uint16_t obj[OAM_LINE_LIMIT] = {0};
  int cnt = collect_sprites(bus, line_y, obj_size, obj);
  uint16_t maxx = TILE_COORD_WRAP;
  uint16_t maxm = 0xFFFF;
  while (cnt--) {
    int midx = -1;
    for (int i = 0; i < OAM_LINE_LIMIT; i++) {
      if (obj[i]) {
        uint8_t sort_x =
            mmu_read_oam(bus->mmu, (uint16_t)(obj[i] - OAM_START + OAM_OFF_X));
        if (sort_x < maxx || (sort_x == maxx && obj[i] < maxm)) {
          if (midx == -1) {
            {
              midx = i;
            }
          } else {
            uint8_t prev = mmu_read_oam(
                bus->mmu, (uint16_t)(obj[midx] - OAM_START + OAM_OFF_X));
            if (sort_x > prev) {
              midx = i;
            }
            if (sort_x == prev && obj[i] > obj[midx]) {
              midx = i;
            }
          }
        }
      }
    }
    if (midx == -1) {
      break;
    }
    uint16_t mem_loc = obj[midx];
    obj[midx] = 0;
    maxx = mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    maxm = mem_loc;
    uint8_t obj_y =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
    int obj_x =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    uint16_t idx =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
    uint8_t flg =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
    obj_y -= OBJ_Y_OFFSET;
    obj_x -= OBJ_X_OFFSET;
    if (obj_size) {
      idx &= OBJ_TALL_TILE_MASK;
    }
    idx *= TILE_BYTES;
    idx += OBJ_TILE_BASE;
    uint8_t flipx = get_bit(flg, OBJ_ATTR_FLIP_X_BIT);
    uint8_t flipy = get_bit(flg, OBJ_ATTR_FLIP_Y_BIT);
    uint8_t line = line_y - obj_y;
    if (obj_size) {
      if (flipy) {
        line = OBJ_TALL_HEIGHT - 1 - line;
      }
    } else {
      if (flipy) {
        line = TILE_HEIGHT - 1 - line;
      }
    }
    line = (uint8_t)((unsigned)line << 1U);
    uint8_t lo_byte = mmu_read_vram_bank0(bus->mmu, idx + line + 0);
    uint8_t hi_byte =
        mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + (uint16_t)line + 1));
    uint8_t pal = get_bit(flg, OBJ_ATTR_PAL_DMG_BIT) ? ppu->obp1 : ppu->obp0;
    for (int x0 = obj_x; x0 < obj_x + TILE_WIDTH; x0++) {
      if (x0 < 0) {
        continue;
      }
      uint8_t posx = (uint8_t)(TILE_WIDTH - 1) - (uint8_t)(x0 - obj_x);
      if (flipx) {
        posx = TILE_WIDTH - 1 - posx;
      }
      uint8_t clr = (uint8_t)(((unsigned)get_bit(hi_byte, posx) << 1U) |
                              (unsigned)get_bit(lo_byte, posx));
      if (get_bit(flg, OBJ_ATTR_PRIORITY_BIT)) {
        if (ppu->dsp[line_y][x0] == palette_color(ppu->bgp, 0)) {
          write_pixel(ppu, line_y, x0, palette_color(pal, clr));
        }
      } else if (clr != 0) {
        {
          write_pixel(ppu, line_y, x0, palette_color(pal, clr));
        }
      }
    }
  }
}

void render_line_dmg(struct Ppu* ppu, struct Bus* bus, uint8_t line_y) {
  if (get_bit(ppu->lcdc, LCDC_BIT_BG_ENABLE)) {
    render_bg_window_dmg(ppu, bus, line_y);
  } else {
    uint8_t blank = palette_color(ppu->bgp, 0);
    for (int col = 0; col < SCRN_WIDTH; col++) {
      write_pixel(ppu, line_y, col, blank);
    }
  }
  if (get_bit(ppu->lcdc, LCDC_BIT_OBJ_ENABLE)) {
    render_sprites_dmg(ppu, bus, line_y);
  }
}
