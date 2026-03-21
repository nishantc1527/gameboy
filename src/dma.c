#include "gbemu/dma.h"

#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/mmu.h"

struct Dma* dma_init(void) {
  struct Dma* d = calloc(1, sizeof(struct Dma));
  d->active = false;
  return d;
}

void dma_free(struct Dma* d) { free(d); }

void dma_trigger(struct Dma* d, struct Bus* bus, uint8_t val) {
  (void)d;
  if (val <= 0xDF) {
    uint16_t src = (uint16_t)((uint16_t)val * 0x100);
    for (uint16_t t = 0; t < 0xA0; t++) {
      mmu_write_oam(bus->mmu, t, bus_read(bus, (uint16_t)(src + t)));
    }
    mmu_write_io(bus->mmu, 0x46, 0xFF);
  }
}

bool dma_blocks_cpu(const struct Dma* d) { return d->active; }
