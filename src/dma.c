#include "gbemu/dma.h"

#include <stdbool.h>
#include <stdint.h>
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

enum {
  DMA_REG_ADDR = 0xFF46,
  DMA_SRC_ADDR_SHIFT = 8,
  DMA_MAX_SRC_PAGE = 0xDF,
  DMA_BYTE_COUNT = 160
};

enum {
  HDMA_SRC_HI_REG = 0xFF51,
  HDMA_SRC_LO_REG = 0xFF52,
  HDMA_DST_HI_REG = 0xFF53,
  HDMA_DST_LO_REG = 0xFF54,
  HDMA_LEN_REG = 0xFF55,
  HDMA_BLOCK_BYTES = 16,
  HDMA_TYPE_BIT = 7,
  HDMA_SRC_LO_MASK = 0xF0,
  HDMA_DST_HI_MASK = 0x1F,
  HDMA_DST_LO_MASK = 0xF0,
  HDMA_DST_WRAP = 0x1FFF,
  HDMA_INACTIVE = 0xFF
};

static uint16_t hdma_calc_src(const struct Dma* dma) {
  return (uint16_t)(((unsigned)dma->hdma1 << 8U) |
                    (unsigned)(dma->hdma2 & HDMA_SRC_LO_MASK));
}

static uint16_t hdma_calc_dst(const struct Dma* dma) {
  return (uint16_t)(VRAM_START |
                    ((unsigned)(dma->hdma3 & HDMA_DST_HI_MASK) << 8U) |
                    (unsigned)(dma->hdma4 & HDMA_DST_LO_MASK));
}

static uint8_t dma_mem_read(struct Bus* bus, uint16_t addr) {
  if (addr < VRAM_START) {
    return mmu_read_rom(bus->mmu, addr);
  }
  if (addr < ERAM_START) {
    return mmu_read_vram(bus->mmu, addr);
  }
  if (addr < WRAM_START) {
    return mmu_read_eram(bus->mmu, addr);
  }
  if (addr < ECHO_START) {
    return mmu_read_wram(bus->mmu, addr);
  }
  return BUS_OPEN_BUS;
}

struct Dma* dma_init(void) {
  struct Dma* dma = calloc(1, sizeof(struct Dma));
  dma->hdma1 = HDMA_INACTIVE;
  dma->hdma2 = HDMA_INACTIVE;
  dma->hdma3 = HDMA_INACTIVE;
  dma->hdma4 = HDMA_INACTIVE;
  dma->hdma5 = HDMA_INACTIVE;
  return dma;
}

void dma_free(struct Dma* dma) { free(dma); }

void dma_trigger(struct Dma* dma, uint8_t val) {
  if (val <= DMA_MAX_SRC_PAGE) {
    dma->src = (uint16_t)((uint16_t)val << DMA_SRC_ADDR_SHIFT);
    dma->pos = 0;
    dma->active = true;
  }
}

void dma_tick(struct Dma* dma, struct Bus* bus, uint8_t cycles) {
  if (!dma->active) {
    return;
  }
  for (uint8_t i = 0; i < cycles && dma->pos < DMA_BYTE_COUNT; i++) {
    uint8_t byte = dma_mem_read(bus, (uint16_t)(dma->src + dma->pos));
    mmu_write_oam(bus->mmu, dma->pos, byte);
    dma->pos++;
  }
  if (dma->pos >= DMA_BYTE_COUNT) {
    dma->active = false;
    dma->pos = 0;
  }
}

bool dma_blocks_cpu(const struct Dma* dma) { return dma->active; }

void dma_notify_hblank(struct Dma* dma) { dma->hdma_block_pending = true; }

bool dma_hdma_block_pending(const struct Dma* dma) {
  return dma->hdma_block_pending;
}

void dma_clear_hdma_block_pending(struct Dma* dma) {
  dma->hdma_block_pending = false;
}

void dma_hdma_write(struct Dma* dma, struct Bus* bus, uint16_t addr,
                    uint8_t val) {
  switch (addr) {
    case HDMA_SRC_HI_REG:
      dma->hdma1 = val;
      return;
    case HDMA_SRC_LO_REG:
      dma->hdma2 = val;
      return;
    case HDMA_DST_HI_REG:
      dma->hdma3 = val;
      return;
    case HDMA_DST_LO_REG:
      dma->hdma4 = val;
      return;
    case HDMA_LEN_REG:
      if (dma->hdma_active && ((unsigned)val & (1U << HDMA_TYPE_BIT)) == 0U) {
        dma->hdma_active = false;
        dma->hdma5 = (uint8_t)((1U << HDMA_TYPE_BIT) | dma->hdma_remaining);
      } else if (((unsigned)val & (1U << HDMA_TYPE_BIT)) == 0U) {
        uint16_t src = hdma_calc_src(dma);
        uint16_t dst = hdma_calc_dst(dma);
        uint16_t blocks = (uint16_t)(((unsigned)val & 0x7FU) + 1U);
        for (uint16_t i = 0; i < blocks * HDMA_BLOCK_BYTES; i++) {
          uint8_t byte = dma_mem_read(bus, (uint16_t)(src + i));
          mmu_write_vram(bus->mmu, (uint16_t)(dst + i), byte);
        }
        dma->hdma5 = HDMA_INACTIVE;
      } else {
        dma->hdma_src = hdma_calc_src(dma);
        dma->hdma_dst = hdma_calc_dst(dma);
        dma->hdma_remaining = (uint8_t)((unsigned)val & 0x7FU);
        dma->hdma_active = true;
        dma->hdma5 = (uint8_t)((unsigned)val & 0x7FU);
      }
      return;
    default:
      return;
  }
}

uint8_t dma_hdma_read(const struct Dma* dma, uint16_t addr) {
  switch (addr) {
    case HDMA_SRC_HI_REG:
      return dma->hdma1;
    case HDMA_SRC_LO_REG:
      return dma->hdma2;
    case HDMA_DST_HI_REG:
      return dma->hdma3;
    case HDMA_DST_LO_REG:
      return dma->hdma4;
    case HDMA_LEN_REG:
      return dma->hdma5;
    default:
      return BUS_OPEN_BUS;
  }
}

void dma_hdma_block(struct Dma* dma, struct Bus* bus) {
  if (!dma->hdma_active) {
    return;
  }
  for (uint16_t i = 0; i < HDMA_BLOCK_BYTES; i++) {
    uint8_t byte = dma_mem_read(bus, (uint16_t)(dma->hdma_src + i));
    mmu_write_vram(bus->mmu, (uint16_t)(dma->hdma_dst + i), byte);
  }
  dma->hdma_src = (uint16_t)(dma->hdma_src + HDMA_BLOCK_BYTES);
  dma->hdma_dst =
      (uint16_t)(VRAM_START | (uint16_t)((dma->hdma_dst + HDMA_BLOCK_BYTES) &
                                         HDMA_DST_WRAP));
  if (dma->hdma_remaining == 0) {
    dma->hdma_active = false;
    dma->hdma5 = HDMA_INACTIVE;
  } else {
    dma->hdma_remaining--;
    dma->hdma5 = dma->hdma_remaining;
  }
}
