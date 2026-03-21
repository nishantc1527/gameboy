#pragma once
#include <stdbool.h>
#include <stdint.h>

struct Dma {
  bool active;
  uint16_t src;
  uint8_t pos;
};

struct Bus;

struct Dma* dma_init(void);
void dma_free(struct Dma* d);
void dma_trigger(struct Dma* d, struct Bus* bus, uint8_t val);
bool dma_blocks_cpu(const struct Dma* d);
