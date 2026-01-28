#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "internal.h"

uint8_t dsp[SCRN_HEIGHT][SCRN_WIDTH];

const uint16_t SCANLINE_LEN = 456;
const uint16_t SCANLINES = 154;

int gt_clr(uint8_t pal, int val) { return (pal >> (val << 1)) & 0b11; }
void w_pxl(int y, int x, int clr) { dsp[y][x] = clr; }

void do_scanline(void) {
  if (get_bit(LCDC, 7)) {
    if (LY < SCRN_HEIGHT) {
      if (get_bit(LCDC, 0)) {
        uint8_t ly = LY;
        int dat_area = get_bit(LCDC, 4);
        int mp_area = get_bit(LCDC, 3);
        uint8_t pal = BGP;
        for (int x = 0; x < SCRN_WIDTH; x++) {
          uint8_t by = (ly + SCY) % 256;
          int tiley = by / 8;
          int offy = by % 8;
          int uint8_ty = offy << 1;
          int bx = (x + SCX) % 256;
          int tilex = bx / 8;
          int offx = bx % 8;
          int idx = (tiley * 32) + tilex;
          if (mp_area == 0)
            idx += 0x9800;
          else
            idx += 0x9C00;
          idx = r_mem(idx);
          if (dat_area == 0) idx = (signed char)idx + 128;
          idx *= 16;
          if (dat_area == 0)
            idx += 0x8800;
          else
            idx += 0x8000;
          uint8_t ls = r_mem(idx + uint8_ty);
          uint8_t ms = r_mem(idx + uint8_ty + 1);
          offx = 7 - offx;
          int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
          w_pxl(ly, x, gt_clr(pal, clr));
        }
        if (get_bit(LCDC, 5)) {
          mp_area = get_bit(LCDC, 6);
          uint8_t wx = WX;
          uint8_t wy = WY;
          if (wx < SCRN_WIDTH + 7 && wy < SCRN_HEIGHT && LY >= wy) {
            wx = wx - 7;
            wy = WIN_CNT;
            int tiley = wy / 8;
            int offy = wy % 8;
            int uint8_ty = offy << 1;
            for (uint8_t x = wx; x < SCRN_WIDTH; x++) {
              uint8_t _wx = x - wx;
              int tilex = _wx / 8;
              int offx = _wx % 8;
              int idx = (tiley * 32) + tilex;
              if (mp_area == 0)
                idx += 0x9800;
              else
                idx += 0x9C00;
              idx = r_mem(idx);
              if (dat_area == 0) idx = (signed char)idx + 128;
              idx *= 16;
              if (dat_area == 0)
                idx += 0x8800;
              else
                idx += 0x8000;
              uint8_t ls = r_mem(idx + uint8_ty);
              uint8_t ms = r_mem(idx + uint8_ty + 1);
              offx = 7 - offx;
              int clr = (get_bit(ms, offx) << 1) | get_bit(ls, offx);
              w_pxl(ly, x, gt_clr(pal, clr));
            }
            WIN_CNT++;
          }
        }
      } else {
        for (int x = 0; x < SCRN_WIDTH; x++) {
          w_pxl(LY, x, CLR_WHT);
        }
      }
      if (get_bit(LCDC, 1)) {
        uint8_t ly = LY;
        uint8_t sz = get_bit(LCDC, 2);
        int cnt = 0;
        uint16_t obj[10] = {0};
        for (uint16_t mem_loc = 0xFE00; mem_loc <= 0xFE9F && cnt < 10;
             mem_loc += 4) {
          int y = r_mem(mem_loc + 0);
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
              uint8_t x = r_mem(obj[i] + 1);
              if (x < maxx || (x == maxx && obj[i] < maxm)) {
                if (midx == -1)
                  midx = i;
                else {
                  uint8_t prev = r_mem(obj[midx] + 1);
                  if (x > prev) midx = i;
                  if (x == prev && obj[i] > obj[midx]) midx = i;
                }
              }
            }
          if (midx == -1) break;
          uint16_t mem_loc = obj[midx];
          obj[midx] = 0;
          maxx = r_mem(mem_loc + 1);
          maxm = mem_loc;
          int y = r_mem(mem_loc + 0);
          int x = r_mem(mem_loc + 1);
          uint16_t idx = r_mem(mem_loc + 2);
          uint8_t flg = r_mem(mem_loc + 3);
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
          line <<= 1;
          uint8_t ls = r_mem(idx + line + 0);
          uint8_t ms = r_mem(idx + line + 1);
          uint8_t pal;
          if (get_bit(flg, 4))
            pal = OBP1;
          else
            pal = OBP0;
          for (int x0 = x; x0 < x + 8; x0++) {
            if (x0 < 0) continue;
            uint8_t posx = 7 - (x0 - x);
            if (flipx) posx = 7 - posx;
            uint8_t clr = (get_bit(ms, posx) << 1) | get_bit(ls, posx);
            if (get_bit(flg, 7)) {
              if (dsp[ly][x0] == gt_clr(BGP, 0))
                w_pxl(ly, x0, gt_clr(pal, clr));
            } else if (clr != 0)
              w_pxl(ly, x0, gt_clr(pal, clr));
          }
        }
      }
    }
    int ly = LY;
    ly++;
    if (ly >= SCANLINES) {
      ly = 0;
      WIN_CNT = 0;
      frame = 1;
    }
    w_mem(0xFF44, ly);
  } else
    w_mem(0xFF44, 0);
}
