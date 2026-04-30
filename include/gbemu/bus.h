#pragma once
#include <stdint.h>

enum {
  VRAM_START = 0x8000,
  VRAM_END = 0x9FFF,
  ERAM_START = 0xA000,
  ERAM_END = 0xBFFF,
  WRAM_START = 0xC000,
  WRAM_END = 0xDFFF,
  ECHO_START = 0xE000,
  ECHO_END = 0xFDFF,
  ECHO_OFFSET = 0x2000,
  OAM_START = 0xFE00,
  OAM_END = 0xFE9F,
  IO_START = 0xFF00,
  IO_END = 0xFF7F,
  HRAM_START = 0xFF80,
  HRAM_END = 0xFFFE,
  IE_REG_ADDR = 0xFFFF
};

enum { BUS_OPEN_BUS = 0xFF };

enum { IF_VALID_MASK = 0x1F, IF_UNUSED_BITS = 0xE0 };

enum {
  INTR_VBLANK = 0,
  INTR_LCD = 1,
  INTR_TIMER = 2,
  INTR_SERIAL = 3,
  INTR_JOYPAD = 4
};

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
void bus_free(struct Bus* bus_ptr);
uint8_t bus_read(struct Bus* bus, uint16_t addr);
void bus_write(struct Bus* bus, uint16_t addr, uint8_t val);
void bus_req_intr(struct Bus* bus, uint8_t intr);
void bus_notify_div_pulse(struct Bus* bus);
