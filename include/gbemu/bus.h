#pragma once
#include <stdint.h>

struct Cpu;
struct Ppu;
struct Apu;
struct Timer;
struct Dma;
struct Joypad;
struct Serial;
struct Mmu;

struct Bus {
  struct Mmu* mmu;
  struct Cpu* cpu;
  struct Ppu* ppu;
  struct Apu* apu;
  struct Timer* timer;
  struct Dma* dma;
  struct Joypad* joypad;
  struct Serial* serial;
};

struct Bus* bus_init(struct Mmu* mmu, struct Cpu* cpu, struct Ppu* ppu,
                     struct Apu* apu, struct Timer* timer, struct Dma* dma,
                     struct Joypad* joypad, struct Serial* serial);
void bus_free(struct Bus* b);
uint8_t bus_read(struct Bus* bus, uint16_t addr);
void bus_write(struct Bus* bus, uint16_t addr, uint8_t val);
void bus_req_intr(struct Bus* bus, uint8_t intr);
