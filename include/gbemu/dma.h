#pragma once
#include <stdint.h>

struct Dma {
  uint8_t active;
  uint16_t src;
  uint8_t pos;
  uint8_t hdma_active;
  uint16_t hdma_src;
  uint16_t hdma_dst;
  uint8_t hdma_remaining;
  uint8_t hdma1, hdma2, hdma3, hdma4;
  uint8_t hdma5;
  uint8_t hdma_block_pending;
};

struct Bus;
struct Mmu;

struct Dma* dma_init(void);
void dma_free(struct Dma* d);
void dma_trigger(struct Dma* d, uint8_t val);
void dma_tick(struct Dma* d, struct Bus* bus, uint8_t cycles);
uint8_t dma_blocks_cpu(const struct Dma* d);
void dma_hdma_write(struct Dma* d, struct Bus* bus, uint16_t addr, uint8_t val);
uint8_t dma_hdma_read(const struct Dma* d, uint16_t addr);
void dma_hdma_block(struct Dma* d, struct Bus* bus);
