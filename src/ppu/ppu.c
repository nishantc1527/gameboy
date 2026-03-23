#include "gbemu/ppu.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/dma.h"
#include "gbemu/util.h"
#include "ppu_private.h"

struct Ppu* ppu_init(void) {
  struct Ppu* ppu = calloc(1, sizeof(struct Ppu));
  return ppu;
}

uint8_t ppu_read(const struct Ppu* ppu, uint16_t addr) {
  switch (addr) {
    case 0xFF40:
      return ppu->lcdc;
    case 0xFF41:
      return ppu->stat | STAT_UNUSED_BIT;
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
      return ppu->bg_pal_ram[ppu->bg_pal_idx & PAL_IDX_MASK];
    case 0xFF6A:
      return ppu->obj_pal_idx;
    case 0xFF6B:
      return ppu->obj_pal_ram[ppu->obj_pal_idx & PAL_IDX_MASK];
    default:
      return 0xFF;
  }
}

void ppu_write(struct Ppu* ppu, uint16_t addr, uint8_t val) {
  switch (addr) {
    case 0xFF40: {
      uint8_t was_on = (ppu->lcdc & (1u << LCDC_BIT_LCD_ENABLE)) != 0;
      uint8_t now_on = (val & (1u << LCDC_BIT_LCD_ENABLE)) != 0;
      if (!was_on && now_on) ppu->lcdc_reenable = true;
      ppu->lcdc = val;
      break;
    }
    case 0xFF41:
      ppu->stat = (ppu->stat & STAT_MODE_PRESERVE) | (val & STAT_WRITABLE_MASK);
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
      uint8_t idx = ppu->bg_pal_idx & PAL_IDX_MASK;
      ppu->bg_pal_ram[idx] = val;
      if (ppu->bg_pal_idx & PAL_AUTO_INC_BIT)
        ppu->bg_pal_idx = PAL_AUTO_INC_BIT | ((idx + 1) & PAL_IDX_MASK);
      break;
    }
    case 0xFF6A:
      ppu->obj_pal_idx = val;
      break;
    case 0xFF6B: {
      uint8_t idx = ppu->obj_pal_idx & PAL_IDX_MASK;
      ppu->obj_pal_ram[idx] = val;
      if (ppu->obj_pal_idx & PAL_AUTO_INC_BIT)
        ppu->obj_pal_idx = PAL_AUTO_INC_BIT | ((idx + 1) & PAL_IDX_MASK);
      break;
    }
    default:
      break;
  }
}

void ppu_post_boot(struct Ppu* ppu, uint8_t cgb_mode) {
  ppu->lcdc = PPU_BOOT_LCDC;
  ppu->stat = PPU_BOOT_STAT;
  ppu->scy = 0x00;
  ppu->scx = 0x00;
  ppu->ly = 0x00;
  ppu->lyc = 0x00;
  ppu->bgp = PPU_BOOT_BGP;
  ppu->wy = 0x00;
  ppu->wx = 0x00;
  (void)cgb_mode;
}

void ppu_tick(struct Ppu* ppu, struct Bus* bus, uint8_t cycles) {
  ppu->scn = (uint16_t)(ppu->scn + cycles);
  if (ppu->scn >= PPU_CYCLES_PER_LINE) {
    do_scanline(ppu, bus);
    ppu->scn -= PPU_CYCLES_PER_LINE;
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
  int prev_mode = stat & PPU_MODE_MASK;
  uint8_t curr_mode;
  uint16_t mode3_end =
      (uint16_t)(PPU_TRANSFER_BASE_END + scx_mode3_penalty[ppu->scx & 7u]);
  if (ppu->scn < PPU_OAM_END_CYCLE)
    curr_mode = PPU_MODE_OAM;
  else if (ppu->scn < mode3_end)
    curr_mode = PPU_MODE_TRANSFER;
  else
    curr_mode = PPU_MODE_HBLANK;
  if (ppu->ly >= PPU_VISIBLE_LINES) curr_mode = PPU_MODE_VBLANK;
  check_interrupt_vblank_lcd(bus, stat, prev_mode, curr_mode);
  if (prev_mode != PPU_MODE_HBLANK && curr_mode == PPU_MODE_HBLANK)
    dma_hdma_block(bus->dma, bus);
  stat &= (uint8_t)~PPU_MODE_MASK;
  stat |= curr_mode;
  if (ppu->ly == ppu->lyc)
    set_bit(&stat, STAT_LYC_FLAG_BIT);
  else
    clear_bit(&stat, STAT_LYC_FLAG_BIT);
  ppu->stat = stat;
}
