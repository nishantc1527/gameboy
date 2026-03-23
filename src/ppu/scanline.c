#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "ppu_private.h"

uint8_t gt_clr(uint8_t pal, int val) {
  return (pal >> (val << 1)) & PPU_MODE_MASK;
}
void w_pxl(struct Ppu* ppu, int y, int x, uint8_t clr) { ppu->dsp[y][x] = clr; }

static void do_scanline_cgb(struct Ppu* ppu, struct Bus* bus) {
  uint8_t ly = ppu->ly;
  uint8_t lcdc = ppu->lcdc;
  uint8_t bg_color_idx[SCRN_WIDTH];
  uint8_t bg_prio_bit[SCRN_WIDTH];
  for (int i = 0; i < SCRN_WIDTH; i++) {
    bg_color_idx[i] = 0;
    bg_prio_bit[i] = 0;
  }
  {
    int dat_area = get_bit(lcdc, LCDC_BIT_TILE_DATA);
    int mp_area = get_bit(lcdc, LCDC_BIT_BG_MAP);
    for (int x = 0; x < SCRN_WIDTH; x++) {
      uint8_t by = (uint8_t)((ly + ppu->scy) % TILE_COORD_WRAP);
      uint8_t tiley = by / TILE_HEIGHT;
      int offy = by % TILE_HEIGHT;
      uint8_t bx = (uint8_t)((x + ppu->scx) % TILE_COORD_WRAP);
      uint8_t tilex = bx / TILE_WIDTH;
      uint8_t offx = bx % TILE_WIDTH;
      uint16_t tile_map_addr =
          (uint16_t)((uint16_t)tiley * TILE_MAP_STRIDE + (uint16_t)tilex);
      tile_map_addr =
          (uint16_t)(tile_map_addr + (mp_area ? TILE_MAP_1 : TILE_MAP_0));
      uint8_t tile_idx_raw = mmu_read_vram_bank0(bus->mmu, tile_map_addr);
      uint8_t attr = mmu_read_vram_bank1(bus->mmu, tile_map_addr);
      uint8_t pal_num = attr & 7;
      uint8_t vram_bank = (attr >> OBJ_ATTR_VRAM_BANK_BIT) & 1;
      uint8_t xflip = (attr >> OBJ_ATTR_FLIP_X_BIT) & 1;
      uint8_t yflip = (attr >> OBJ_ATTR_FLIP_Y_BIT) & 1;
      uint8_t bg_prio = (attr >> OBJ_ATTR_PRIORITY_BIT) & 1;
      int tile_offy = yflip ? (TILE_HEIGHT - 1 - offy) : offy;
      int ty = tile_offy << 1;
      uint16_t idx = tile_idx_raw;
      if (dat_area == 0) idx = (uint16_t)((int8_t)idx + 128);
      idx = (uint16_t)(idx * TILE_BYTES);
      idx = (uint16_t)(idx + (dat_area ? TILE_DATA_LO : TILE_DATA_HI));
      uint8_t ls, ms;
      if (vram_bank == 0) {
        ls = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + ty));
        ms = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + ty + 1));
      } else {
        ls = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + ty));
        ms = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + ty + 1));
      }
      uint8_t bit_pos = xflip ? offx : (uint8_t)(TILE_WIDTH - 1 - offx);
      int clr = (get_bit(ms, bit_pos) << 1) | get_bit(ls, bit_pos);
      bg_color_idx[x] = (uint8_t)clr;
      bg_prio_bit[x] = bg_prio;
      uint8_t lo = ppu->bg_pal_ram[pal_num * 8 + clr * 2];
      uint8_t hi = ppu->bg_pal_ram[pal_num * 8 + clr * 2 + 1];
      ppu->cgb_dsp[ly][x] = (uint16_t)(lo | ((uint16_t)hi << 8));
    }
    if (get_bit(lcdc, LCDC_BIT_WIN_ENABLE)) {
      int win_mp = get_bit(lcdc, LCDC_BIT_WIN_MAP);
      uint8_t wx = ppu->wx;
      if (wx < SCRN_WIDTH + 7 && ppu->wy_triggered) {
        wx -= 7;
        uint8_t win_ly = ppu->win_cnt;
        uint8_t tiley = win_ly / TILE_HEIGHT;
        int offy = win_ly % TILE_HEIGHT;
        for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
          uint8_t _wx = (uint8_t)(x - wx);
          uint8_t tilex = _wx / TILE_WIDTH;
          uint8_t offx = _wx % TILE_WIDTH;
          uint16_t tile_map_addr =
              (uint16_t)((uint16_t)tiley * TILE_MAP_STRIDE + (uint16_t)tilex);
          tile_map_addr =
              (uint16_t)(tile_map_addr + (win_mp ? TILE_MAP_1 : TILE_MAP_0));
          uint8_t tile_idx_raw = mmu_read_vram_bank0(bus->mmu, tile_map_addr);
          uint8_t attr = mmu_read_vram_bank1(bus->mmu, tile_map_addr);
          uint8_t pal_num = attr & 7;
          uint8_t vram_bank = (attr >> OBJ_ATTR_VRAM_BANK_BIT) & 1;
          uint8_t xflip = (attr >> OBJ_ATTR_FLIP_X_BIT) & 1;
          uint8_t yflip = (attr >> OBJ_ATTR_FLIP_Y_BIT) & 1;
          uint8_t bg_prio = (attr >> OBJ_ATTR_PRIORITY_BIT) & 1;
          int tile_offy = yflip ? (TILE_HEIGHT - 1 - offy) : offy;
          int ty = tile_offy << 1;
          uint16_t idx = tile_idx_raw;
          if (dat_area == 0) idx = (uint16_t)((int8_t)idx + 128);
          idx = (uint16_t)(idx * TILE_BYTES);
          idx = (uint16_t)(idx + (dat_area ? TILE_DATA_LO : TILE_DATA_HI));
          uint8_t ls, ms;
          if (vram_bank == 0) {
            ls = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + ty));
            ms = mmu_read_vram_bank0(bus->mmu, (uint16_t)(idx + ty + 1));
          } else {
            ls = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + ty));
            ms = mmu_read_vram_bank1(bus->mmu, (uint16_t)(idx + ty + 1));
          }
          uint8_t bit_pos = xflip ? offx : (uint8_t)(TILE_WIDTH - 1 - offx);
          int clr = (get_bit(ms, bit_pos) << 1) | get_bit(ls, bit_pos);
          bg_color_idx[x] = (uint8_t)clr;
          bg_prio_bit[x] = bg_prio;
          uint8_t lo = ppu->bg_pal_ram[pal_num * 8 + clr * 2];
          uint8_t hi = ppu->bg_pal_ram[pal_num * 8 + clr * 2 + 1];
          ppu->cgb_dsp[ly][x] = (uint16_t)(lo | ((uint16_t)hi << 8));
        }
        ppu->win_cnt++;
      }
    }
  }
  if (get_bit(lcdc, LCDC_BIT_OBJ_ENABLE)) {
    uint8_t sz = get_bit(lcdc, LCDC_BIT_OBJ_SIZE);
    int cnt = 0;
    uint16_t obj[OAM_LINE_LIMIT] = {0};
    for (uint16_t mem_loc = OAM_START;
         mem_loc <= OAM_END && cnt < OAM_LINE_LIMIT;
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
    for (int i = cnt - 1; i >= 0; i--) {
      uint16_t mem_loc = obj[i];
      uint8_t y =
          mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
      int x =
          mmu_read_oam(bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
      uint16_t tile_idx = mmu_read_oam(
          bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
      uint8_t flg = mmu_read_oam(
          bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
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
        int bg_wins = 0;
        if (!get_bit(lcdc, LCDC_BIT_BG_ENABLE)) {
          bg_wins = 0;
        } else if (bg_prio_bit[x0] && bg_color_idx[x0] != 0) {
          bg_wins = 1;
        } else if (obj_prio && bg_color_idx[x0] != 0) {
          bg_wins = 1;
        }
        if (!bg_wins) {
          uint8_t lo = ppu->obj_pal_ram[pal_num * 8 + clr * 2];
          uint8_t hi = ppu->obj_pal_ram[pal_num * 8 + clr * 2 + 1];
          ppu->cgb_dsp[ly][x0] = (uint16_t)(lo | ((uint16_t)hi << 8));
        }
      }
    }
  }
}

void do_scanline(struct Ppu* ppu, struct Bus* bus) {
  if (get_bit(ppu->lcdc, LCDC_BIT_LCD_ENABLE)) {
    if (ppu->ly < PPU_VISIBLE_LINES) {
      if (ppu->cgb_mode) {
        do_scanline_cgb(ppu, bus);
        ppu->ly++;
        if (ppu->ly >= PPU_TOTAL_LINES) {
          ppu->ly = 0;
          ppu->win_cnt = 0;
          ppu->wy_triggered = 0;
          ppu->frame_ready = true;
        }
        return;
      }
      if (get_bit(ppu->lcdc, LCDC_BIT_BG_ENABLE)) {
        uint8_t ly = ppu->ly;
        int dat_area = get_bit(ppu->lcdc, LCDC_BIT_TILE_DATA);
        int mp_area = get_bit(ppu->lcdc, LCDC_BIT_BG_MAP);
        uint8_t pal = ppu->bgp;
        for (int x = 0; x < SCRN_WIDTH; x++) {
          uint8_t by = (ly + ppu->scy) % TILE_COORD_WRAP;
          uint8_t tiley = by / (uint8_t)TILE_HEIGHT;
          int offy = by % TILE_HEIGHT;
          int ty = offy << 1;
          uint8_t bx = (uint8_t)((x + ppu->scx) % TILE_COORD_WRAP);
          uint8_t tilex = bx / TILE_WIDTH;
          uint8_t offx = bx % TILE_WIDTH;
          uint16_t idx =
              (uint16_t)((uint16_t)tiley * (uint16_t)TILE_MAP_STRIDE) +
              (uint16_t)tilex;
          if (mp_area == 0)
            idx += TILE_MAP_0;
          else
            idx += TILE_MAP_1;
          idx = mmu_read_vram(bus->mmu, idx);
          if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
          idx *= TILE_BYTES;
          if (dat_area == 0)
            idx += TILE_DATA_HI;
          else
            idx += TILE_DATA_LO;
          uint8_t ls = mmu_read_vram(bus->mmu, (uint16_t)(idx + (uint16_t)ty));
          uint8_t ms =
              mmu_read_vram(bus->mmu, (uint16_t)(idx + (uint16_t)ty + 1));
          offx = TILE_WIDTH - 1 - offx;
          int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
          w_pxl(ppu, ly, x, gt_clr(pal, clr));
        }
        if (get_bit(ppu->lcdc, LCDC_BIT_WIN_ENABLE)) {
          mp_area = get_bit(ppu->lcdc, LCDC_BIT_WIN_MAP);
          uint8_t wx = ppu->wx;
          if (wx < SCRN_WIDTH + 7 && ppu->wy_triggered) {
            wx = wx - 7;
            uint8_t wy = ppu->win_cnt;
            uint8_t tiley = wy / TILE_HEIGHT;
            int offy = wy % TILE_HEIGHT;
            int ty = offy << 1;
            for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
              uint8_t _wx = x - wx;
              uint8_t tilex = _wx / TILE_WIDTH;
              uint8_t offx = _wx % TILE_WIDTH;
              uint16_t idx =
                  (uint16_t)((uint16_t)tiley * (uint16_t)TILE_MAP_STRIDE) +
                  (uint16_t)tilex;
              if (mp_area == 0)
                idx += TILE_MAP_0;
              else
                idx += TILE_MAP_1;
              idx = mmu_read_vram(bus->mmu, idx);
              if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
              idx *= TILE_BYTES;
              if (dat_area == 0)
                idx += TILE_DATA_HI;
              else
                idx += TILE_DATA_LO;
              uint8_t ls = mmu_read_vram(bus->mmu, idx + (uint16_t)ty);
              uint8_t ms = mmu_read_vram(
                  bus->mmu, (uint16_t)(idx + (uint16_t)ty + (uint16_t)1));
              offx = TILE_WIDTH - 1 - offx;
              int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
              w_pxl(ppu, ly, x, gt_clr(pal, clr));
            }
            ppu->win_cnt++;
          }
        }
      } else {
        for (int x = 0; x < SCRN_WIDTH; x++) {
          w_pxl(ppu, ppu->ly, x, CLR_WHT);
        }
      }
      if (get_bit(ppu->lcdc, LCDC_BIT_OBJ_ENABLE)) {
        uint8_t ly = ppu->ly;
        uint8_t sz = get_bit(ppu->lcdc, LCDC_BIT_OBJ_SIZE);
        int cnt = 0;
        uint16_t obj[OAM_LINE_LIMIT] = {0};
        for (uint16_t mem_loc = OAM_START;
             mem_loc <= OAM_END && cnt < OAM_LINE_LIMIT;
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
        uint16_t maxx = TILE_COORD_WRAP;
        uint16_t maxm = 0xFFFF;
        while (cnt--) {
          int midx = -1;
          for (int i = 0; i < OAM_LINE_LIMIT; i++)
            if (obj[i]) {
              uint8_t x = mmu_read_oam(
                  bus->mmu, (uint16_t)(obj[i] - OAM_START + OAM_OFF_X));
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
          maxx = mmu_read_oam(bus->mmu,
                              (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
          maxm = mem_loc;
          uint8_t y = mmu_read_oam(bus->mmu,
                                   (uint16_t)(mem_loc - OAM_START + OAM_OFF_Y));
          int x = mmu_read_oam(bus->mmu,
                               (uint16_t)(mem_loc - OAM_START + OAM_OFF_X));
          uint16_t idx = mmu_read_oam(
              bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_TILE));
          uint8_t flg = mmu_read_oam(
              bus->mmu, (uint16_t)(mem_loc - OAM_START + OAM_OFF_ATTR));
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
          uint8_t ms = mmu_read_vram_bank0(
              bus->mmu, (uint16_t)(idx + (uint16_t)line + 1));
          uint8_t pal;
          if (get_bit(flg, OBJ_ATTR_PAL_DMG_BIT))
            pal = ppu->obp1;
          else
            pal = ppu->obp0;
          for (int x0 = x; x0 < x + TILE_WIDTH; x0++) {
            if (x0 < 0) continue;
            uint8_t posx = (uint8_t)(TILE_WIDTH - 1) - (uint8_t)(x0 - x);
            if (flipx) posx = TILE_WIDTH - 1 - posx;
            uint8_t clr = (uint8_t)(get_bit(ms, posx) << 1) | get_bit(ls, posx);
            if (get_bit(flg, OBJ_ATTR_PRIORITY_BIT)) {
              if (ppu->dsp[ly][x0] == gt_clr(ppu->bgp, 0))
                w_pxl(ppu, ly, x0, gt_clr(pal, clr));
            } else if (clr != 0)
              w_pxl(ppu, ly, x0, gt_clr(pal, clr));
          }
        }
      }
    }
    ppu->ly++;
    if (ppu->ly >= PPU_TOTAL_LINES) {
      ppu->ly = 0;
      ppu->win_cnt = 0;
      ppu->wy_triggered = 0;
      ppu->frame_ready = true;
    }
  } else {
    ppu->ly = 0;
    ppu->win_cnt = 0;
    ppu->wy_triggered = 0;
    ppu->off_scn++;
    if (ppu->off_scn >= PPU_TOTAL_LINES) {
      ppu->off_scn = 0;
      ppu->frame_ready = true;
    }
  }
}
