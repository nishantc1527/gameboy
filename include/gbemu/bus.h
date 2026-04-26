#pragma once
#include <stdint.h>

#define VRAM_START 0x8000
#define VRAM_END 0x9FFF
#define ERAM_START 0xA000
#define ERAM_END 0xBFFF
#define WRAM_START 0xC000
#define WRAM_END 0xDFFF
#define ECHO_START 0xE000
#define ECHO_END 0xFDFF
#define ECHO_OFFSET 0x2000
#define OAM_START 0xFE00
#define OAM_END 0xFE9F
#define IO_START 0xFF00
#define IO_END 0xFF7F
#define HRAM_START 0xFF80
#define HRAM_END 0xFFFE
#define IE_REG_ADDR 0xFFFF

#define BUS_OPEN_BUS 0xFF

#define IF_VALID_MASK 0x1F
#define IF_UNUSED_BITS 0xE0

#define INTR_VBLANK 0
#define INTR_LCD 1
#define INTR_TIMER 2
#define INTR_SERIAL 3
#define INTR_JOYPAD 4

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
void bus_notify_div_pulse(struct Bus* bus);
