#include <stdint.h>

#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "internal.h"

uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];

const uint16_t SCANLINE_LEN = 456;
const uint16_t SCANLINES = 154;

uint8_t gt_clr(uint8_t pal, int val) { return (pal >> (val << 1)) & 0b11; }
void w_pxl(int y, int x, uint8_t clr) { dsp[y][x] = clr; }

void do_scanline(void) {
  if (gt(mmu_r_mem(mmu, LCDC), 7)) {
    if (mmu_r_mem(mmu, LY) < SCRN_HEIGHT) {
      if (gt(mmu_r_mem(mmu, LCDC), 0)) {
        uint8_t ly = mmu_r_mem(mmu, LY);
        int dat_area = gt(mmu_r_mem(mmu, LCDC), 4);
        int mp_area = gt(mmu_r_mem(mmu, LCDC), 3);
        uint8_t pal = mmu_r_mem(mmu, BGP);
        for (int x = 0; x < SCRN_WIDTH; x++) {
          uint8_t by = (ly + mmu_r_mem(mmu, SCY)) % 256;
          uint8_t tiley = by / (uint8_t)8;
          int offy = by % 8;
          int ty = offy << 1;
          uint8_t bx = (uint8_t)((x + mmu_r_mem(mmu, SCX)) % 256);
          uint8_t tilex = bx / 8;
          uint8_t offx = bx % 8;
          uint16_t idx =
              (uint16_t)((uint16_t)tiley * (uint16_t)32) + (uint16_t)tilex;
          if (mp_area == 0)
            idx += 0x9800;
          else
            idx += 0x9C00;
          idx = mmu_r_mem(mmu, idx);
          if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
          idx *= 16;
          if (dat_area == 0)
            idx += 0x8800;
          else
            idx += 0x8000;
          uint8_t ls = mmu_r_mem(mmu, (uint16_t)(idx + (uint16_t)ty));
          uint8_t ms = mmu_r_mem(mmu, (uint16_t)(idx + (uint16_t)ty + 1));
          offx = 7 - offx;
          int clr = (gt(ms, offx) << 1) | gt(ls, offx);
          w_pxl(ly, x, gt_clr(pal, clr));
        }
        if (gt(mmu_r_mem(mmu, LCDC), 5)) {
          mp_area = gt(mmu_r_mem(mmu, LCDC), 6);
          uint8_t wx = mmu_r_mem(mmu, WX);
          uint8_t wy = mmu_r_mem(mmu, WY);
          if (wx < SCRN_WIDTH + 7 && wy < SCRN_HEIGHT &&
              mmu_r_mem(mmu, LY) >= wy) {
            wx = wx - 7;
            wy = WIN_CNT;
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
              idx = mmu_r_mem(mmu, idx);
              if (dat_area == 0) idx = (uint16_t)((int8_t)idx + (uint16_t)128);
              idx *= 16;
              if (dat_area == 0)
                idx += 0x8800;
              else
                idx += 0x8000;
              uint8_t ls = mmu_r_mem(mmu, idx + (uint16_t)ty);
              uint8_t ms =
                  mmu_r_mem(mmu, (uint16_t)(idx + (uint16_t)ty + (uint16_t)1));
              offx = 7 - offx;
              int clr = (gt(ms, offx) << 1) | gt(ls, offx);
              w_pxl(ly, x, gt_clr(pal, clr));
            }
            WIN_CNT++;
          }
        }
      } else {
        for (int x = 0; x < SCRN_WIDTH; x++) {
          w_pxl(mmu_r_mem(mmu, LY), x, CLR_WHT);
        }
      }
      if (gt(mmu_r_mem(mmu, LCDC), 1)) {
        uint8_t ly = mmu_r_mem(mmu, LY);
        uint8_t sz = gt(mmu_r_mem(mmu, LCDC), 2);
        int cnt = 0;
        uint16_t obj[10] = {0};
        for (uint16_t mem_loc = 0xFE00; mem_loc <= 0xFE9F && cnt < 10;
             mem_loc += 4) {
          int y = mmu_r_mem(mmu, mem_loc + 0);
          y -= 16;
          if (ly < y) continue;
          if (sz) {
            if (ly >= y + 16) {
              continue;
            }
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
              uint8_t x = mmu_r_mem(mmu, obj[i] + 1);
              if (x < maxx || (x == maxx && obj[i] < maxm)) {
                if (midx == -1)
                  midx = i;
                else {
                  uint8_t prev = mmu_r_mem(mmu, obj[midx] + 1);
                  if (x > prev) midx = i;
                  if (x == prev && obj[i] > obj[midx]) midx = i;
                }
              }
            }
          if (midx == -1) break;
          uint16_t mem_loc = obj[midx];
          obj[midx] = 0;
          maxx = mmu_r_mem(mmu, mem_loc + 1);
          maxm = mem_loc;
          uint8_t y = mmu_r_mem(mmu, mem_loc + 0);
          int x = mmu_r_mem(mmu, mem_loc + 1);
          uint16_t idx = mmu_r_mem(mmu, mem_loc + 2);
          uint8_t flg = mmu_r_mem(mmu, mem_loc + 3);
          y -= 16;
          x -= 8;
          if (sz) idx &= 0xFE;
          idx *= 16;
          idx += 0x8000;
          uint8_t flipx = gt(flg, 5);
          uint8_t flipy = gt(flg, 6);
          uint8_t line = ly - y;
          if (sz) {
            if (flipy) line = 15 - line;
          } else {
            if (flipy) line = 7 - line;
          }
          line = (uint8_t)(line << 1);
          uint8_t ls = mmu_r_mem(mmu, idx + line + 0);
          uint8_t ms = mmu_r_mem(mmu, (uint16_t)(idx + (uint16_t)line + 1));
          uint8_t pal;
          if (gt(flg, 4))
            pal = mmu_r_mem(mmu, OBP1);
          else
            pal = mmu_r_mem(mmu, OBP0);
          for (int x0 = x; x0 < x + 8; x0++) {
            if (x0 < 0) continue;
            uint8_t posx = (uint8_t)7 - (uint8_t)(x0 - x);
            if (flipx) posx = 7 - posx;
            uint8_t clr = (uint8_t)(gt(ms, posx) << 1) | gt(ls, posx);
            if (gt(flg, 7)) {
              if (dsp[ly][x0] == gt_clr(mmu_r_mem(mmu, BGP), 0))
                w_pxl(ly, x0, gt_clr(pal, clr));
            } else if (clr != 0)
              w_pxl(ly, x0, gt_clr(pal, clr));
          }
        }
      }
    }
    uint8_t ly = mmu_r_mem(mmu, LY);
    ly++;
    if (ly >= SCANLINES) {
      ly = 0;
      WIN_CNT = 0;
      frame = 1;
    }
    mmu_w_mem(mmu, 0xFF44, ly);
  } else
    mmu_w_mem(mmu, 0xFF44, 0);
}
