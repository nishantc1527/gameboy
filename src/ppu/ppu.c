#include "gbemu/ppu.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/dma.h"
#include "gbemu/util.h"

#define INTR_VBLANK 0
#define INTR_LCD 1

struct Ppu* init_ppu(void) {
  struct Ppu* ppu = calloc(1, sizeof(struct Ppu));
  return ppu;
}

uint8_t ppu_read(const struct Ppu* ppu, uint16_t addr) {
  switch (addr) {
    case 0xFF40:
      return ppu->lcdc;
    case 0xFF41:
      return ppu->stat | 0x80;
    case 0xFF42:
      return ppu->scy;
    case 0xFF43:
      return ppu->scx;
    case 0xFF44:
      return ppu->ly;
    case 0xFF45:
      return ppu->lyc;
    case 0xFF47:
      return ppu->bgp;
    case 0xFF48:
      return ppu->obp0;
    case 0xFF49:
      return ppu->obp1;
    case 0xFF4A:
      return ppu->wy;
    case 0xFF4B:
      return ppu->wx;
    case 0xFF68:
      return ppu->bg_pal_idx;
    case 0xFF69:
      return ppu->bg_pal_ram[ppu->bg_pal_idx & 0x3F];
    case 0xFF6A:
      return ppu->obj_pal_idx;
    case 0xFF6B:
      return ppu->obj_pal_ram[ppu->obj_pal_idx & 0x3F];
    default:
      return 0xFF;
  }
}

void ppu_write(struct Ppu* ppu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF40: {
      uint8_t was_on = (ppu->lcdc & 0x80u) != 0;
      uint8_t now_on = (val & 0x80u) != 0;
      if (!was_on && now_on) ppu->lcdc_reenable = true;
      ppu->lcdc = val;
      break;
    }
    case 0xFF41:
      ppu->stat = (ppu->stat & 0x07) | (val & 0x78);
      break;
    case 0xFF42:
      ppu->scy = val;
      break;
    case 0xFF43:
      ppu->scx = val;
      break;
    case 0xFF44:
      break;
    case 0xFF45:
      ppu->lyc = val;
      break;
    case 0xFF47:
      ppu->bgp = val;
      break;
    case 0xFF48:
      ppu->obp0 = val;
      break;
    case 0xFF49:
      ppu->obp1 = val;
      break;
    case 0xFF4A:
      ppu->wy = val;
      break;
    case 0xFF4B:
      ppu->wx = val;
      break;
    case 0xFF68:
      ppu->bg_pal_idx = val;
      break;
    case 0xFF69: {
      uint8_t idx = ppu->bg_pal_idx & 0x3F;
      ppu->bg_pal_ram[idx] = val;
      if (ppu->bg_pal_idx & 0x80) ppu->bg_pal_idx = 0x80 | ((idx + 1) & 0x3F);
      break;
    }
    case 0xFF6A:
      ppu->obj_pal_idx = val;
      break;
    case 0xFF6B: {
      uint8_t idx = ppu->obj_pal_idx & 0x3F;
      ppu->obj_pal_ram[idx] = val;
      if (ppu->obj_pal_idx & 0x80) ppu->obj_pal_idx = 0x80 | ((idx + 1) & 0x3F);
      break;
    }
    default:
      break;
  }
}

void ppu_post_boot(struct Ppu* ppu, uint8_t cgb_mode) {
  ppu->lcdc = 0x91;
  ppu->stat = 0x85;
  ppu->scy = 0x00;
  ppu->scx = 0x00;
  ppu->ly = 0x00;
  ppu->lyc = 0x00;
  ppu->bgp = 0xFC;
  ppu->wy = 0x00;
  ppu->wx = 0x00;
  (void)cgb_mode;
}

void ppu_tick(struct Ppu* ppu, struct Bus* bus, uint8_t cycles) {
  ppu->scn = (uint16_t)(ppu->scn + cycles);
  if (ppu->scn >= SCANLINE_LEN) {
    do_scanline(ppu, bus);
    ppu->scn -= SCANLINE_LEN;
  }
  if (ppu->lcdc_reenable) {
    ppu->lcdc_reenable = false;
    ppu->scn = 4;
    ppu->ly = 0;
  }
  update_lcd(ppu, bus);
}

static const uint16_t scx_mode3_penalty[8] = {0, 0, 0, 0, 4, 4, 4, 8};

void update_lcd(struct Ppu* ppu, struct Bus* bus) {
  uint8_t stat = ppu->stat;
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  uint16_t mode3_end = (uint16_t)(252u + scx_mode3_penalty[ppu->scx & 7u]);
  if (ppu->scn < 80)
    curr_mode = 2;
  else if (ppu->scn < mode3_end)
    curr_mode = 3;
  else
    curr_mode = 0;
  if (ppu->ly >= SCRN_HEIGHT) curr_mode = 1;
  check_interrupt_vblank_lcd(bus, stat, prev_mode, curr_mode);
  if (prev_mode != 0 && curr_mode == 0) dma_hdma_block(bus->dma, bus);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (ppu->ly == ppu->lyc)
    set_bit(&stat, 2);
  else
    clear_bit(&stat, 2);
  ppu->stat = stat;
}
