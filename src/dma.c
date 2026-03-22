#include "gbemu/dma.h"

#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"

static uint8_t dma_bus_read(struct Bus* bus, uint16_t addr) {
  if (addr < 0x8000) return mmu_read_rom(bus->mmu, addr);
  if (addr < 0xA000) return mmu_read_vram(bus->mmu, addr);
  if (addr < 0xC000) return mmu_read_eram(bus->mmu, addr);
  if (addr < 0xE000) return mmu_read_wram(bus->mmu, addr);
  return 0xFF;
}

struct Dma* dma_init(void) {
  struct Dma* d = calloc(1, sizeof(struct Dma));
  d->hdma1 = 0xFF;
  d->hdma2 = 0xFF;
  d->hdma3 = 0xFF;
  d->hdma4 = 0xFF;
  d->hdma5 = 0xFF;
  return d;
}

void dma_free(struct Dma* d) { free(d); }

void dma_trigger(struct Dma* d, uint8_t val) {
  if (val <= 0xDF) {
    d->src = (uint16_t)((uint16_t)val * 0x100);
    d->pos = 0;
    d->active = 1;
  }
}

void dma_tick(struct Dma* d, struct Bus* bus, uint8_t cycles) {
  if (!d->active) return;
  for (uint8_t i = 0; i < cycles && d->pos < 160; i++) {
    uint8_t byte = dma_bus_read(bus, (uint16_t)(d->src + d->pos));
    mmu_write_oam(bus->mmu, d->pos, byte);
    d->pos++;
  }
  if (d->pos >= 160) {
    d->active = 0;
    d->pos = 0;
  }
}

uint8_t dma_blocks_cpu(const struct Dma* d) { return d->active; }

void dma_hdma_write(struct Dma* d, struct Bus* bus, uint16_t addr,
                    uint8_t val) {
  switch (addr) {
    case 0xFF51:
      d->hdma1 = val;
      return;
    case 0xFF52:
      d->hdma2 = val;
      return;
    case 0xFF53:
      d->hdma3 = val;
      return;
    case 0xFF54:
      d->hdma4 = val;
      return;
    case 0xFF55:
      if (d->hdma_active && (val & 0x80) == 0) {
        d->hdma_active = false;
        d->hdma5 = 0x80u | d->hdma_remaining;
      } else if ((val & 0x80) == 0) {
        uint16_t src = ((uint16_t)d->hdma1 << 8) | (uint16_t)(d->hdma2 & 0xF0);
        uint16_t dst = 0x8000u | ((uint16_t)(d->hdma3 & 0x1F) << 8) |
                       (uint16_t)(d->hdma4 & 0xF0);
        uint16_t blocks = (uint16_t)(val & 0x7Fu) + 1u;
        for (uint16_t i = 0; i < blocks * 0x10u; i++) {
          uint8_t byte = dma_bus_read(bus, (uint16_t)(src + i));
          mmu_write_vram(bus->mmu, (uint16_t)(dst + i), byte);
        }
        d->hdma5 = 0xFF;
      } else {
        d->hdma_src = ((uint16_t)d->hdma1 << 8) | (uint16_t)(d->hdma2 & 0xF0);
        d->hdma_dst = 0x8000u | ((uint16_t)(d->hdma3 & 0x1F) << 8) |
                      (uint16_t)(d->hdma4 & 0xF0);
        d->hdma_remaining = val & 0x7Fu;
        d->hdma_active = true;
        d->hdma5 = val & 0x7Fu;
      }
      return;
    default:
      return;
  }
}

uint8_t dma_hdma_read(const struct Dma* d, uint16_t addr) {
  switch (addr) {
    case 0xFF51:
      return d->hdma1;
    case 0xFF52:
      return d->hdma2;
    case 0xFF53:
      return d->hdma3;
    case 0xFF54:
      return d->hdma4;
    case 0xFF55:
      return d->hdma5;
    default:
      return 0xFF;
  }
}

void dma_hdma_block(struct Dma* d, struct Bus* bus) {
  if (!d->hdma_active) return;
  for (uint16_t i = 0; i < 0x10u; i++) {
    uint8_t byte = dma_bus_read(bus, (uint16_t)(d->hdma_src + i));
    mmu_write_vram(bus->mmu, (uint16_t)(d->hdma_dst + i), byte);
  }
  d->hdma_src = (uint16_t)(d->hdma_src + 0x10u);
  d->hdma_dst = 0x8000u | (uint16_t)((d->hdma_dst + 0x10u) & 0x1FFFu);
  if (d->hdma_remaining == 0) {
    d->hdma_active = false;
    d->hdma5 = 0xFF;
  } else {
    d->hdma_remaining--;
    d->hdma5 = d->hdma_remaining;
  }
}
