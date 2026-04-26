#include "gbemu/dma.h"

#include <stdbool.h>
#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"

struct Dma {
  bool active;
  uint16_t src;
  uint8_t pos;
  bool hdma_active;
  uint16_t hdma_src;
  uint16_t hdma_dst;
  uint8_t hdma_remaining;
  uint8_t hdma1, hdma2, hdma3, hdma4;
  uint8_t hdma5;
  bool hdma_block_pending;
};

#define DMA_REG_ADDR 0xFF46
#define DMA_SRC_ADDR_SHIFT 8
#define DMA_MAX_SRC_PAGE 0xDF
#define DMA_BYTE_COUNT 160

#define HDMA_SRC_HI_REG 0xFF51
#define HDMA_SRC_LO_REG 0xFF52
#define HDMA_DST_HI_REG 0xFF53
#define HDMA_DST_LO_REG 0xFF54
#define HDMA_LEN_REG 0xFF55
#define HDMA_BLOCK_BYTES 16
#define HDMA_TYPE_BIT 7
#define HDMA_SRC_LO_MASK 0xF0
#define HDMA_DST_HI_MASK 0x1F
#define HDMA_DST_LO_MASK 0xF0
#define HDMA_DST_WRAP 0x1FFF
#define HDMA_INACTIVE 0xFF

static uint16_t hdma_calc_src(const struct Dma* d) {
  return ((uint16_t)d->hdma1 << 8) | (uint16_t)(d->hdma2 & HDMA_SRC_LO_MASK);
}

static uint16_t hdma_calc_dst(const struct Dma* d) {
  return VRAM_START | ((uint16_t)(d->hdma3 & HDMA_DST_HI_MASK) << 8) |
         (uint16_t)(d->hdma4 & HDMA_DST_LO_MASK);
}

static uint8_t dma_mem_read(struct Bus* bus, uint16_t addr) {
  if (addr < VRAM_START) return mmu_read_rom(bus->mmu, addr);
  if (addr < ERAM_START) return mmu_read_vram(bus->mmu, addr);
  if (addr < WRAM_START) return mmu_read_eram(bus->mmu, addr);
  if (addr < ECHO_START) return mmu_read_wram(bus->mmu, addr);
  return BUS_OPEN_BUS;
}

struct Dma* dma_init(void) {
  struct Dma* d = calloc(1, sizeof(struct Dma));
  d->hdma1 = HDMA_INACTIVE;
  d->hdma2 = HDMA_INACTIVE;
  d->hdma3 = HDMA_INACTIVE;
  d->hdma4 = HDMA_INACTIVE;
  d->hdma5 = HDMA_INACTIVE;
  return d;
}

void dma_free(struct Dma* d) { free(d); }

void dma_trigger(struct Dma* d, uint8_t val) {
  if (val <= DMA_MAX_SRC_PAGE) {
    d->src = (uint16_t)((uint16_t)val << DMA_SRC_ADDR_SHIFT);
    d->pos = 0;
    d->active = true;
  }
}

void dma_tick(struct Dma* d, struct Bus* bus, uint8_t cycles) {
  if (!d->active) return;
  for (uint8_t i = 0; i < cycles && d->pos < DMA_BYTE_COUNT; i++) {
    uint8_t byte = dma_mem_read(bus, (uint16_t)(d->src + d->pos));
    mmu_write_oam(bus->mmu, d->pos, byte);
    d->pos++;
  }
  if (d->pos >= DMA_BYTE_COUNT) {
    d->active = false;
    d->pos = 0;
  }
}

bool dma_blocks_cpu(const struct Dma* d) { return d->active; }

void dma_notify_hblank(struct Dma* d) { d->hdma_block_pending = true; }

bool dma_hdma_block_pending(const struct Dma* d) {
  return d->hdma_block_pending;
}

void dma_clear_hdma_block_pending(struct Dma* d) {
  d->hdma_block_pending = false;
}

void dma_hdma_write(struct Dma* d, struct Bus* bus, uint16_t addr,
                    uint8_t val) {
  switch (addr) {
    case HDMA_SRC_HI_REG:
      d->hdma1 = val;
      return;
    case HDMA_SRC_LO_REG:
      d->hdma2 = val;
      return;
    case HDMA_DST_HI_REG:
      d->hdma3 = val;
      return;
    case HDMA_DST_LO_REG:
      d->hdma4 = val;
      return;
    case HDMA_LEN_REG:
      if (d->hdma_active && (val & (1u << HDMA_TYPE_BIT)) == 0) {
        d->hdma_active = false;
        d->hdma5 = (1u << HDMA_TYPE_BIT) | d->hdma_remaining;
      } else if ((val & (1u << HDMA_TYPE_BIT)) == 0) {
        uint16_t src = hdma_calc_src(d);
        uint16_t dst = hdma_calc_dst(d);
        uint16_t blocks = (uint16_t)(val & 0x7Fu) + 1u;
        for (uint16_t i = 0; i < blocks * HDMA_BLOCK_BYTES; i++) {
          uint8_t byte = dma_mem_read(bus, (uint16_t)(src + i));
          mmu_write_vram(bus->mmu, (uint16_t)(dst + i), byte);
        }
        d->hdma5 = HDMA_INACTIVE;
      } else {
        d->hdma_src = hdma_calc_src(d);
        d->hdma_dst = hdma_calc_dst(d);
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
    case HDMA_SRC_HI_REG:
      return d->hdma1;
    case HDMA_SRC_LO_REG:
      return d->hdma2;
    case HDMA_DST_HI_REG:
      return d->hdma3;
    case HDMA_DST_LO_REG:
      return d->hdma4;
    case HDMA_LEN_REG:
      return d->hdma5;
    default:
      return BUS_OPEN_BUS;
  }
}

void dma_hdma_block(struct Dma* d, struct Bus* bus) {
  if (!d->hdma_active) return;
  for (uint16_t i = 0; i < HDMA_BLOCK_BYTES; i++) {
    uint8_t byte = dma_mem_read(bus, (uint16_t)(d->hdma_src + i));
    mmu_write_vram(bus->mmu, (uint16_t)(d->hdma_dst + i), byte);
  }
  d->hdma_src = (uint16_t)(d->hdma_src + HDMA_BLOCK_BYTES);
  d->hdma_dst =
      VRAM_START | (uint16_t)((d->hdma_dst + HDMA_BLOCK_BYTES) & HDMA_DST_WRAP);
  if (d->hdma_remaining == 0) {
    d->hdma_active = false;
    d->hdma5 = HDMA_INACTIVE;
  } else {
    d->hdma_remaining--;
    d->hdma5 = d->hdma_remaining;
  }
}
