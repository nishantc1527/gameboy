#pragma once
#include <stdbool.h>
#include <stdint.h>

struct Bus;
struct Mmu;
struct Dma;

struct Dma* dma_init(void);
void dma_free(struct Dma* dma);
void dma_trigger(struct Dma* dma, uint8_t val);
void dma_tick(struct Dma* dma, struct Bus* bus, uint8_t cycles);
bool dma_blocks_cpu(const struct Dma* dma);
void dma_hdma_write(struct Dma* dma, struct Bus* bus, uint16_t addr,
                    uint8_t val);
uint8_t dma_hdma_read(const struct Dma* dma, uint16_t addr);
void dma_hdma_block(struct Dma* dma, struct Bus* bus);
void dma_notify_hblank(struct Dma* dma);
bool dma_hdma_block_pending(const struct Dma* dma);
void dma_clear_hdma_block_pending(struct Dma* dma);
