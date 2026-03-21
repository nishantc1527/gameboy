#include <stdint.h>

#include "gbemu/bus.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "internal.h"
#include "rust.h"

const uint16_t SCANLINE_LEN = 456;
const uint16_t SCANLINES = 154;

uint8_t gt_clr(uint8_t pal, int val) { return (pal >> (val << 1)) & 0b11; }
void w_pxl(struct Ppu* ppu, int y, int x, uint8_t clr) { ppu->dsp[y][x] = clr; }

static void do_scanline_cgb(struct Ppu* ppu, struct Bus* bus) {
  uint8_t ly = bus_read(bus, LY);
  uint8_t lcdc = bus_read(bus, LCDC);
  uint8_t bg_color_idx[SCRN_WIDTH];
  uint8_t bg_prio_bit[SCRN_WIDTH];
  for (int i = 0; i < SCRN_WIDTH; i++) {
    bg_color_idx[i] = 0;
    bg_prio_bit[i] = 0;
  }
  {
    int dat_area = get_bit(lcdc, 4);
    int mp_area = get_bit(lcdc, 3);
    for (int x = 0; x < SCRN_WIDTH; x++) {
      uint8_t by = (uint8_t)((ly + bus_read(bus, SCY)) % 256);
      uint8_t tiley = by / 8;
      int offy = by % 8;
      uint8_t bx = (uint8_t)((x + bus_read(bus, SCX)) % 256);
      uint8_t tilex = bx / 8;
      uint8_t offx = bx % 8;
      uint16_t tile_map_addr =
          (uint16_t)((uint16_t)tiley * 32 + (uint16_t)tilex);
      tile_map_addr = (uint16_t)(tile_map_addr + (mp_area ? 0x9C00 : 0x9800));
      uint8_t tile_idx_raw = mmu_r_mem_raw(bus->mmu, tile_map_addr);
      uint8_t attr = mmu_get_vram_bank1_byte(bus->mmu, tile_map_addr);
      uint8_t pal_num = attr & 7;
      uint8_t vram_bank = (attr >> 3) & 1;
      uint8_t xflip = (attr >> 5) & 1;
      uint8_t yflip = (attr >> 6) & 1;
      uint8_t bg_prio = (attr >> 7) & 1;
      int tile_offy = yflip ? (7 - offy) : offy;
      int ty = tile_offy << 1;
      uint16_t idx = tile_idx_raw;
      if (dat_area == 0) idx = (uint16_t)((int8_t)idx + 128);
      idx = (uint16_t)(idx * 16);
      idx = (uint16_t)(idx + (dat_area ? 0x8000 : 0x8800));
      uint8_t ls, ms;
      if (vram_bank == 0) {
        ls = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + ty));
        ms = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + ty + 1));
      } else {
        ls = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + ty));
        ms = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + ty + 1));
      }
      uint8_t bit_pos = xflip ? offx : (uint8_t)(7 - offx);
      int clr = (get_bit(ms, bit_pos) << 1) | get_bit(ls, bit_pos);
      bg_color_idx[x] = (uint8_t)clr;
      bg_prio_bit[x] = bg_prio;
      uint8_t lo =
          mmu_get_bg_pal_byte(bus->mmu, (uint8_t)(pal_num * 8 + clr * 2));
      uint8_t hi =
          mmu_get_bg_pal_byte(bus->mmu, (uint8_t)(pal_num * 8 + clr * 2 + 1));
      ppu->cgb_dsp[ly][x] = (uint16_t)(lo | ((uint16_t)hi << 8));
    }
    if (get_bit(lcdc, 5)) {
      int win_mp = get_bit(lcdc, 6);
      uint8_t wx = bus_read(bus, WX);
      uint8_t wy = bus_read(bus, WY);
      if (wx < SCRN_WIDTH + 7 && wy < SCRN_HEIGHT && ly >= wy) {
        wx -= 7;
        uint8_t win_ly = ppu->WIN_CNT;
        uint8_t tiley = win_ly / 8;
        int offy = win_ly % 8;
        for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
          uint8_t _wx = (uint8_t)(x - wx);
          uint8_t tilex = _wx / 8;
          uint8_t offx = _wx % 8;
          uint16_t tile_map_addr =
              (uint16_t)((uint16_t)tiley * 32 + (uint16_t)tilex);
          tile_map_addr =
              (uint16_t)(tile_map_addr + (win_mp ? 0x9C00 : 0x9800));
          uint8_t tile_idx_raw = mmu_r_mem_raw(bus->mmu, tile_map_addr);
          uint8_t attr = mmu_get_vram_bank1_byte(bus->mmu, tile_map_addr);
          uint8_t pal_num = attr & 7;
          uint8_t vram_bank = (attr >> 3) & 1;
          uint8_t xflip = (attr >> 5) & 1;
          uint8_t yflip = (attr >> 6) & 1;
          uint8_t bg_prio = (attr >> 7) & 1;
          int tile_offy = yflip ? (7 - offy) : offy;
          int ty = tile_offy << 1;
          uint16_t idx = tile_idx_raw;
          if (dat_area == 0) idx = (uint16_t)((int8_t)idx + 128);
          idx = (uint16_t)(idx * 16);
          idx = (uint16_t)(idx + (dat_area ? 0x8000 : 0x8800));
          uint8_t ls, ms;
          if (vram_bank == 0) {
            ls = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + ty));
            ms = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + ty + 1));
          } else {
            ls = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + ty));
            ms = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + ty + 1));
          }
          uint8_t bit_pos = xflip ? offx : (uint8_t)(7 - offx);
          int clr = (get_bit(ms, bit_pos) << 1) | get_bit(ls, bit_pos);
          bg_color_idx[x] = (uint8_t)clr;
          bg_prio_bit[x] = bg_prio;
          uint8_t lo =
              mmu_get_bg_pal_byte(bus->mmu, (uint8_t)(pal_num * 8 + clr * 2));
          uint8_t hi = mmu_get_bg_pal_byte(
              bus->mmu, (uint8_t)(pal_num * 8 + clr * 2 + 1));
          ppu->cgb_dsp[ly][x] = (uint16_t)(lo | ((uint16_t)hi << 8));
        }
        ppu->WIN_CNT++;
      }
    }
  }
  if (get_bit(lcdc, 1)) {
    uint8_t sz = get_bit(lcdc, 2);
    int cnt = 0;
    uint16_t obj[10] = {0};
    for (uint16_t mem_loc = 0xFE00; mem_loc <= 0xFE9F && cnt < 10;
         mem_loc += 4) {
      int y = bus_read(bus, mem_loc + 0);
      y -= 16;
      if (ly < y) continue;
      if (sz) {
        if (ly >= y + 16) continue;
      } else if (ly >= y + 8)
        continue;
      obj[cnt++] = mem_loc;
    }
    for (int i = cnt - 1; i >= 0; i--) {
      uint16_t mem_loc = obj[i];
      uint8_t y = bus_read(bus, mem_loc + 0);
      int x = bus_read(bus, mem_loc + 1);
      uint16_t tile_idx = bus_read(bus, mem_loc + 2);
      uint8_t flg = bus_read(bus, mem_loc + 3);
      y -= 16;
      x -= 8;
      if (sz) tile_idx &= 0xFE;
      uint8_t vram_bank = (flg >> 3) & 1;
      uint8_t pal_num = ppu->cgb_compat ? ((flg >> 4) & 1) : (flg & 7);
      uint8_t flipx = get_bit(flg, 5);
      uint8_t flipy = get_bit(flg, 6);
      uint8_t obj_prio = get_bit(flg, 7);
      uint16_t idx = (uint16_t)(tile_idx * 16 + 0x8000);
      uint8_t line = (uint8_t)(ly - y);
      if (sz) {
        if (flipy) line = 15 - line;
      } else {
        if (flipy) line = 7 - line;
      }
      line = (uint8_t)(line << 1);
      uint8_t ls, ms;
      if (vram_bank == 0) {
        ls = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + line));
        ms = mmu_r_mem_raw(bus->mmu, (uint16_t)(idx + line + 1));
      } else {
        ls = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + line));
        ms = mmu_get_vram_bank1_byte(bus->mmu, (uint16_t)(idx + line + 1));
      }
      for (int x0 = x; x0 < x + 8; x0++) {
        if (x0 < 0 || x0 >= SCRN_WIDTH) continue;
        uint8_t posx = (uint8_t)(x0 - x);
        if (!flipx) posx = 7 - posx;
        uint8_t clr = (uint8_t)((get_bit(ms, posx) << 1) | get_bit(ls, posx));
        if (clr == 0) continue;
        int bg_wins = 0;
        if (!get_bit(lcdc, 0)) {
          bg_wins = 0;
        } else if (bg_prio_bit[x0] && bg_color_idx[x0] != 0) {
          bg_wins = 1;
        } else if (obj_prio && bg_color_idx[x0] != 0) {
          bg_wins = 1;
        }
        if (!bg_wins) {
          uint8_t lo =
              mmu_get_obj_pal_byte(bus->mmu, (uint8_t)(pal_num * 8 + clr * 2));
          uint8_t hi = mmu_get_obj_pal_byte(
              bus->mmu, (uint8_t)(pal_num * 8 + clr * 2 + 1));
          ppu->cgb_dsp[ly][x0] = (uint16_t)(lo | ((uint16_t)hi << 8));
        }
      }
    }
  }
}

void do_scanline(struct Ppu* ppu,
                 struct Bus* bus) {  // TODO magic numbers, split up
  if (get_bit(bus_read(bus, LCDC), 7)) {
    if (bus_read(bus, LY) < SCRN_HEIGHT) {
      if (ppu->cgb_mode) {
        do_scanline_cgb(ppu, bus);
        uint8_t ly = bus_read(bus, LY);
        ly++;
        if (ly >= SCANLINES) {
          ly = 0;
          ppu->WIN_CNT = 0;
          ppu->frame = 1;
        }
        bus_write(bus, 0xFF44, ly);
        return;
      }
      if (get_bit(bus_read(bus, LCDC), 0)) {
        uint8_t ly = bus_read(bus, LY);
        int dat_area = get_bit(bus_read(bus, LCDC), 4);
        int mp_area = get_bit(bus_read(bus, LCDC), 3);
        uint8_t pal = bus_read(bus, BGP);
        for (int x = 0; x < SCRN_WIDTH; x++) {
          uint8_t by = (ly + bus_read(bus, SCY)) % 256;
          uint8_t tiley = by / (uint8_t)8;
          int offy = by % 8;
          int ty = offy << 1;
          uint8_t bx = (uint8_t)((x + bus_read(bus, SCX)) % 256);
          uint8_t tilex = bx / 8;
          uint8_t offx = bx % 8;
          uint16_t idx =
              (uint16_t)((uint16_t)tiley * (uint16_t)32) + (uint16_t)tilex;
          if (mp_area == 0)
            idx += 0x9800;
          else
            idx += 0x9C00;
          idx = bus_read(bus, idx);
          if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
          idx *= 16;
          if (dat_area == 0)
            idx += 0x8800;
          else
            idx += 0x8000;
          uint8_t ls = bus_read(bus, (uint16_t)(idx + (uint16_t)ty));
          uint8_t ms = bus_read(bus, (uint16_t)(idx + (uint16_t)ty + 1));
          offx = 7 - offx;
          int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
          w_pxl(ppu, ly, x, gt_clr(pal, clr));
        }
        if (get_bit(bus_read(bus, LCDC), 5)) {
          mp_area = get_bit(bus_read(bus, LCDC), 6);
          uint8_t wx = bus_read(bus, WX);
          uint8_t wy = bus_read(bus, WY);
          if (wx < SCRN_WIDTH + 7 && wy < SCRN_HEIGHT &&
              bus_read(bus, LY) >= wy) {
            wx = wx - 7;
            wy = ppu->WIN_CNT;
            uint8_t tiley = wy / 8;
            int offy = wy % 8;
            int ty = offy << 1;
            for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
              uint8_t _wx = x - wx;
              uint8_t tilex = _wx / 8;
              uint8_t offx = _wx % 8;
              uint16_t idx =
                  (uint16_t)((uint16_t)tiley * (uint16_t)32) + (uint16_t)tilex;
              if (mp_area == 0)
                idx += 0x9800;
              else
                idx += 0x9C00;
              idx = bus_read(bus, idx);
              if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
              idx *= 16;
              if (dat_area == 0)
                idx += 0x8800;
              else
                idx += 0x8000;
              uint8_t ls = bus_read(bus, idx + (uint16_t)ty);
              uint8_t ms =
                  bus_read(bus, (uint16_t)(idx + (uint16_t)ty + (uint16_t)1));
              offx = 7 - offx;
              int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
              w_pxl(ppu, ly, x, gt_clr(pal, clr));
            }
            ppu->WIN_CNT++;
          }
        }
      } else {
        for (int x = 0; x < SCRN_WIDTH; x++) {
          w_pxl(ppu, bus_read(bus, LY), x, CLR_WHT);
        }
      }
      if (get_bit(bus_read(bus, LCDC), 1)) {
        uint8_t ly = bus_read(bus, LY);
        uint8_t sz = get_bit(bus_read(bus, LCDC), 2);
        int cnt = 0;
        uint16_t obj[10] = {0};
        for (uint16_t mem_loc = 0xFE00; mem_loc <= 0xFE9F && cnt < 10;
             mem_loc += 4) {
          int y = bus_read(bus, mem_loc + 0);
          y -= 16;
          if (ly < y) continue;
          if (sz) {
            if (ly >= y + 16) continue;
          } else if (ly >= y + 8)
            continue;
          obj[cnt++] = mem_loc;
        }
        uint16_t maxx = 0x0100;
        uint16_t maxm = 0xFFFF;
        while (cnt--) {
          int midx = -1;
          for (int i = 0; i < 10; i++)
            if (obj[i]) {
              uint8_t x = bus_read(bus, obj[i] + 1);
              if (x < maxx || (x == maxx && obj[i] < maxm)) {
                if (midx == -1)
                  midx = i;
                else {
                  uint8_t prev = bus_read(bus, obj[midx] + 1);
                  if (x > prev) midx = i;
                  if (x == prev && obj[i] > obj[midx]) midx = i;
                }
              }
            }
          if (midx == -1) break;
          uint16_t mem_loc = obj[midx];
          obj[midx] = 0;
          maxx = bus_read(bus, mem_loc + 1);
          maxm = mem_loc;
          uint8_t y = bus_read(bus, mem_loc + 0);
          int x = bus_read(bus, mem_loc + 1);
          uint16_t idx = bus_read(bus, mem_loc + 2);
          uint8_t flg = bus_read(bus, mem_loc + 3);
          y -= 16;
          x -= 8;
          if (sz) idx &= 0xFE;
          idx *= 16;
          idx += 0x8000;
          uint8_t flipx = get_bit(flg, 5);
          uint8_t flipy = get_bit(flg, 6);
          uint8_t line = ly - y;
          if (sz) {
            if (flipy) line = 15 - line;
          } else {
            if (flipy) line = 7 - line;
          }
          line = (uint8_t)(line << 1);
          uint8_t ls = bus_read(bus, idx + line + 0);
          uint8_t ms = bus_read(bus, (uint16_t)(idx + (uint16_t)line + 1));
          uint8_t pal;
          if (get_bit(flg, 4))
            pal = bus_read(bus, OBP1);
          else
            pal = bus_read(bus, OBP0);
          for (int x0 = x; x0 < x + 8; x0++) {
            if (x0 < 0) continue;
            uint8_t posx = (uint8_t)7 - (uint8_t)(x0 - x);
            if (flipx) posx = 7 - posx;
            uint8_t clr = (uint8_t)(get_bit(ms, posx) << 1) | get_bit(ls, posx);
            if (get_bit(flg, 7)) {
              if (ppu->dsp[ly][x0] == gt_clr(bus_read(bus, BGP), 0))
                w_pxl(ppu, ly, x0, gt_clr(pal, clr));
            } else if (clr != 0)
              w_pxl(ppu, ly, x0, gt_clr(pal, clr));
          }
        }
      }
    }
    uint8_t ly = bus_read(bus, LY);
    ly++;
    if (ly >= SCANLINES) {
      ly = 0;
      ppu->WIN_CNT = 0;
      ppu->frame = 1;
    }
    bus_write(bus, 0xFF44, ly);
  } else {
    bus_write(bus, 0xFF44, 0);
    ppu->WIN_CNT = 0;
    ppu->off_scn++;
    if (ppu->off_scn >= SCANLINES) {
      ppu->off_scn = 0;
      ppu->frame = 1;
    }
  }
}
