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

static void cgb_fifo_clear(CgbFifo* que) {
  que->head = 0;
  que->size = 0;
}

static void cgb_fifo_push(CgbFifo* que, uint8_t lo_byte, uint8_t hi_byte,
                          uint8_t pal_num, uint8_t bg_prio, uint8_t xflip) {
  for (int i = 0; i < 8; i++) {
    int bit = xflip ? i : (7 - i);
    que->buf[(unsigned)(que->head + que->size++) &
             (unsigned)(FIFO_CAPACITY - 1)] = (CgbBgPixel){
        .color = (uint8_t)((((unsigned)hi_byte >> (unsigned)bit) & 1U) << 1U |
                           (((unsigned)lo_byte >> (unsigned)bit) & 1U)),
        .pal_num = pal_num,
        .bg_prio = bg_prio,
    };
  }
}

static CgbBgPixel cgb_fifo_pop(CgbFifo* que) {
  CgbBgPixel pixel =
      que->buf[(unsigned)que->head & (unsigned)(FIFO_CAPACITY - 1)];
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
  uint8_t tile_idx;
  uint8_t pal_num;
  uint8_t vram_bank;
  uint8_t xflip;
  uint8_t yflip;
  uint8_t bg_prio;
  uint8_t lo, hi;
} CgbFetcher;

static void cgb_fetcher_init(CgbFetcher* ftc, int tile_x, int tiley, int offy,
                             uint16_t map, int dat_area) {
  ftc->step = 0;
  ftc->tx = (int)((unsigned)tile_x & (unsigned)(TILE_MAP_STRIDE - 1));
  ftc->tiley = tiley;
  ftc->offy = offy;
  ftc->map = map;
  ftc->dat_area = dat_area;
  ftc->tile_idx = ftc->pal_num = ftc->vram_bank = 0;
  ftc->xflip = ftc->yflip = ftc->bg_prio = ftc->lo = ftc->hi = 0;
}

static void cgb_fetcher_tick(CgbFetcher* ftc, CgbFifo* que, struct Bus* bus) {
  switch (ftc->step) {
    case 0:
      ftc->step = 1;
      break;
    case 1: {
      uint16_t offs = (uint16_t)(((uint16_t)ftc->tiley * TILE_MAP_STRIDE) +
                                 (uint16_t)ftc->tx);
      uint16_t vaddr = (uint16_t)(ftc->map + offs);
      ftc->tile_idx = mmu_read_vram_bank0(bus->mmu, vaddr);
      uint8_t attr = mmu_read_vram_bank1(bus->mmu, vaddr);
      ftc->pal_num = (uint8_t)((unsigned)attr & 7U);
      ftc->vram_bank =
          (uint8_t)(((unsigned)attr >> OBJ_ATTR_VRAM_BANK_BIT) & 1U);
      ftc->xflip = (uint8_t)(((unsigned)attr >> OBJ_ATTR_FLIP_X_BIT) & 1U);
      ftc->yflip = (uint8_t)(((unsigned)attr >> OBJ_ATTR_FLIP_Y_BIT) & 1U);
      ftc->bg_prio = (uint8_t)(((unsigned)attr >> OBJ_ATTR_PRIORITY_BIT) & 1U);
      ftc->step = 2;
      break;
    }
    case 2:
      ftc->step = 3;
      break;
    case 3: {
      int row = ftc->yflip ? (TILE_HEIGHT - 1 - ftc->offy) : ftc->offy;
      uint16_t addr = tile_data_addr(ftc->tile_idx, ftc->dat_area);
      uint16_t off = (uint16_t)(addr + (uint16_t)(row << 1));
      ftc->lo = ftc->vram_bank ? mmu_read_vram_bank1(bus->mmu, off)
                               : mmu_read_vram_bank0(bus->mmu, off);
      ftc->step = 4;
      break;
    }
    case 4:
      ftc->step = 5;
      break;
    case 5: {
      int row = ftc->yflip ? (TILE_HEIGHT - 1 - ftc->offy) : ftc->offy;
      uint16_t addr = tile_data_addr(ftc->tile_idx, ftc->dat_area);
      uint16_t off = (uint16_t)(addr + (uint16_t)(row << 1) + 1U);
      ftc->hi = ftc->vram_bank ? mmu_read_vram_bank1(bus->mmu, off)
                               : mmu_read_vram_bank0(bus->mmu, off);
      ftc->step = 6;
      break;
    }
    case 6:
      if (que->size == 0) {
        cgb_fifo_push(que, ftc->lo, ftc->hi, ftc->pal_num, ftc->bg_prio,
                      ftc->xflip);
        ftc->tx =
            (int)(((unsigned)ftc->tx + 1U) & (unsigned)(TILE_MAP_STRIDE - 1));
        ftc->step = 0;
      }
      break;
    default:
      break;
  }
}

static void render_sprites_cgb(struct Ppu* ppu, struct Bus* bus, uint8_t line_y,
                               const uint8_t* bg_color_idx,
                               const uint8_t* bg_prio_bit) {
  uint8_t lcdc = ppu->lcdc;
  uint8_t obj_size = get_bit(lcdc, LCDC_BIT_OBJ_SIZE);
  uint16_t obj[OAM_LINE_LIMIT] = {0};
  int cnt = collect_sprites(bus, line_y, obj_size, obj);
  for (int i = cnt - 1; i >= 0; i--) {
    uint16_t mem_loc = obj[i];
    uint8_t obj_y =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
    int obj_x =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
    uint16_t tile_idx =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
    uint8_t flg =
        mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
    obj_y -= OBJ_Y_OFFSET;
    obj_x -= OBJ_X_OFFSET;
    if (obj_size) {
      tile_idx = (uint16_t)((unsigned)tile_idx & OBJ_TALL_TILE_MASK);
    }
    uint8_t vram_bank =
        (uint8_t)(((unsigned)flg >> OBJ_ATTR_VRAM_BANK_BIT) & 1U);
    uint8_t pal_num =
        (int)ppu->cgb_compat
            ? (uint8_t)(((unsigned)flg >> OBJ_ATTR_PAL_DMG_BIT) & 1U)
            : (uint8_t)((unsigned)flg & 7U);
    uint8_t flipx = get_bit(flg, OBJ_ATTR_FLIP_X_BIT);
    uint8_t flipy = get_bit(flg, OBJ_ATTR_FLIP_Y_BIT);
    uint8_t obj_prio = get_bit(flg, OBJ_ATTR_PRIORITY_BIT);
    uint16_t idx = (uint16_t)((tile_idx * TILE_BYTES) + OBJ_TILE_BASE);
    uint8_t line = (uint8_t)(line_y - obj_y);
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
    uint8_t lo_byte = 0;
    uint8_t hi_byte = 0;
    if (vram_bank == 0) {
      lo_byte = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + line));
      hi_byte = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + line + 1));
    } else {
      lo_byte = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + line));
      hi_byte = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + line + 1));
    }
    for (int x0 = obj_x; x0 < obj_x + TILE_WIDTH; x0++) {
      if (x0 < 0 || x0 >= SCRN_WIDTH) {
        continue;
      }
      uint8_t posx = (uint8_t)(x0 - obj_x);
      if (!flipx) {
        posx = TILE_WIDTH - 1 - posx;
      }
      uint8_t clr = (uint8_t)(((unsigned)get_bit(hi_byte, posx) << 1U) |
                              (unsigned)get_bit(lo_byte, posx));
      if (clr == 0) {
        continue;
      }
      int bg_wins = get_bit(lcdc, LCDC_BIT_BG_ENABLE) &&
                    bg_color_idx[x0] != 0 && (bg_prio_bit[x0] || obj_prio);
      if (!bg_wins) {
        uint8_t pal_lo = ppu->obj_pal_ram[(pal_num * 8) + (clr * 2)];
        uint8_t pal_hi = ppu->obj_pal_ram[(pal_num * 8) + (clr * 2) + 1];
        ppu->cgb_dsp[line_y][x0] =
            (uint16_t)((unsigned)pal_lo | ((unsigned)pal_hi << 8U));
      }
    }
  }
}

void render_line_cgb(struct Ppu* ppu, struct Bus* bus) {
  uint8_t line_y = ppu->ly;
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
  CgbFetcher fetcher;
  cgb_fetcher_init(&fetcher, init_tx, bg_tiley, bg_offy, bg_map, dat_area);
  CgbFifo fifo;
  cgb_fifo_clear(&fifo);
  int discard = (int)((unsigned)scx & 7U);
  int px_out = 0;
  uint8_t bg_color_idx[SCRN_WIDTH];
  uint8_t bg_prio_bit[SCRN_WIDTH];
  while (px_out < SCRN_WIDTH) {
    if (win_enabled && !win_started && discard == 0 && px_out == win_x) {
      cgb_fifo_clear(&fifo);
      cgb_fetcher_init(&fetcher, 0, win_tiley, win_offy, win_map, dat_area);
      win_started = 1;
    }
    cgb_fetcher_tick(&fetcher, &fifo, bus);
    if (fifo.size > 0) {
      CgbBgPixel pixel = cgb_fifo_pop(&fifo);
      if (discard > 0) {
        discard--;
      } else {
        bg_color_idx[px_out] = pixel.color;
        bg_prio_bit[px_out] = pixel.bg_prio;
        uint8_t pal_lo =
            ppu->bg_pal_ram[(pixel.pal_num * 8) + (pixel.color * 2)];
        uint8_t pal_hi =
            ppu->bg_pal_ram[(pixel.pal_num * 8) + (pixel.color * 2) + 1];
        ppu->cgb_dsp[line_y][px_out] =
            (uint16_t)((unsigned)pal_lo | ((unsigned)pal_hi << 8U));
        px_out++;
      }
    }
  }
  if (win_started) {
    ppu->window_line++;
  }
  if (get_bit(lcdc, LCDC_BIT_OBJ_ENABLE)) {
    render_sprites_cgb(ppu, bus, line_y, bg_color_idx, bg_prio_bit);
  }
}
