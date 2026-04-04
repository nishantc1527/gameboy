#pragma once

#include <stdint.h>

#define GBEMU_VERSION "1.0.0"

struct GBemu {
  struct Apu* apu;
  struct Cpu* cpu;
  struct Mmu* mmu;
  struct Ppu* ppu;
  struct Timer* timer;
  struct Dma* dma;
  struct Joypad* joypad;
  struct Serial* serial;
  struct Bus* bus;
  char* rom_name;
  uint8_t disassemble_enable;
  uint8_t paused;
  uint8_t fast_forward;
  uint16_t watch_addrs[8];
  uint8_t watch_count;
  uint64_t total_cycles;
  uint64_t total_frames;
};

struct GBemu* gbemu_init(char* rom_name, uint8_t disassemble_enable,
                         const uint16_t* watch_addrs, uint8_t watch_count);
int gbemu_step_frame(struct GBemu* gb);
void gbemu_free(struct GBemu* gb);
void gbemu_reset(struct GBemu* gb);
