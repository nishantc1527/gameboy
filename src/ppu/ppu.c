#include "gbemu/ppu.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/dma.h"
#include "gbemu/util.h"
#include "ppu_private.h"

struct Ppu* ppu_init(uint8_t cgb_mode, uint8_t cgb_compat) {
  struct Ppu* ppu = calloc(1, sizeof(struct Ppu));
  ppu->cgb_mode = cgb_mode;
  ppu->cgb_compat = cgb_compat;
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
      if (was_on && !now_on) ppu->window_line = 0;
      if (!was_on && now_on) ppu->lcd_turning_on = true;
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

void ppu_post_boot(struct Ppu* ppu) {
  ppu->lcdc = PPU_BOOT_LCDC;
  ppu->stat = PPU_BOOT_STAT;
  ppu->scy = 0x00;
  ppu->scx = 0x00;
  ppu->ly = 0x00;
  ppu->lyc = 0x00;
  ppu->bgp = PPU_BOOT_BGP;
  ppu->wy = 0x00;
  ppu->wx = 0x00;
}

uint8_t ppu_frame_ready(const struct Ppu* ppu) { return ppu->frame_ready; }

void ppu_begin_frame(struct Ppu* ppu) { ppu->frame_ready = 0; }

uint8_t ppu_blocks_vram(const struct Ppu* ppu) {
  return get_bit(ppu->lcdc, LCDC_BIT_LCD_ENABLE) &&
         (ppu->stat & PPU_MODE_MASK) == PPU_MODE_TRANSFER;
}

uint8_t ppu_blocks_oam(const struct Ppu* ppu) {
  uint8_t mode = ppu->stat & PPU_MODE_MASK;
  return get_bit(ppu->lcdc, LCDC_BIT_LCD_ENABLE) &&
         (mode == PPU_MODE_OAM || mode == PPU_MODE_TRANSFER);
}

void ppu_tick(struct Ppu* ppu, struct Bus* bus, uint8_t cycles) {
  ppu->line_cycles = (uint16_t)(ppu->line_cycles + cycles);
  if (ppu->line_cycles >= PPU_CYCLES_PER_LINE) {
    ppu_render_line(ppu, bus);
    ppu->line_cycles -= PPU_CYCLES_PER_LINE;
  }
  if (ppu->lcd_turning_on) {
    ppu->lcd_turning_on = false;
    ppu->line_cycles = 4;
    ppu->ly = 0;
  }
  ppu_update_mode(ppu, bus);
}

static void ppu_check_stat_irq(struct Ppu* ppu, struct Bus* bus,
                               uint8_t curr_mode) {
  uint8_t lyc_match = (ppu->ly == ppu->lyc);
  uint8_t new_line =
      (curr_mode == PPU_MODE_HBLANK &&
       get_bit(ppu->stat, STAT_INT_HBLANK_BIT)) ||
      (curr_mode == PPU_MODE_VBLANK &&
       get_bit(ppu->stat, STAT_INT_VBLANK_BIT)) ||
      (curr_mode == PPU_MODE_OAM && get_bit(ppu->stat, STAT_INT_OAM_BIT)) ||
      (lyc_match && get_bit(ppu->stat, STAT_INT_LYC_BIT));
  if (!ppu->stat_irq_line && new_line) bus_req_intr(bus, INTR_LCD);
  ppu->stat_irq_line = new_line;
}

static const uint16_t scx_mode3_penalty[8] = {0, 0, 0, 0, 4, 4, 4, 8};

void ppu_update_mode(struct Ppu* ppu, struct Bus* bus) {
  uint8_t stat = ppu->stat;
  uint8_t prev_mode = stat & PPU_MODE_MASK;
  uint8_t curr_mode;
  uint16_t mode3_end =
      (uint16_t)(PPU_TRANSFER_BASE_END + scx_mode3_penalty[ppu->scx & 7u]);
  if (ppu->line_cycles < PPU_OAM_END_CYCLE)
    curr_mode = PPU_MODE_OAM;
  else if (ppu->line_cycles < mode3_end)
    curr_mode = PPU_MODE_TRANSFER;
  else
    curr_mode = PPU_MODE_HBLANK;
  if (ppu->ly >= PPU_VISIBLE_LINES) curr_mode = PPU_MODE_VBLANK;
  if (prev_mode != PPU_MODE_VBLANK && curr_mode == PPU_MODE_VBLANK)
    bus_req_intr(bus, INTR_VBLANK);
  if (prev_mode != PPU_MODE_OAM && curr_mode == PPU_MODE_OAM &&
      ppu->ly < PPU_VISIBLE_LINES && ppu->wy == ppu->ly)
    ppu->wy_triggered = 1;
  ppu_check_stat_irq(ppu, bus, curr_mode);
  if (prev_mode != PPU_MODE_HBLANK && curr_mode == PPU_MODE_HBLANK)
    bus->dma->hdma_block_pending = 1;
  stat &= (uint8_t)~PPU_MODE_MASK;
  stat |= curr_mode;
  if (ppu->ly == ppu->lyc)
    set_bit(&stat, STAT_LYC_FLAG_BIT);
  else
    clear_bit(&stat, STAT_LYC_FLAG_BIT);
  ppu->stat = stat;
}
