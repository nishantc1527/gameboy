#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"

typedef struct {
  struct Apu* apu;
  struct Cpu* cpu;
  Mmu* mmu;
  struct Ppu* ppu;
  char* rom_name;
  int test_category;
  uint8_t disassemble_enable;
  uint8_t bdone;
  uint16_t watch_addrs[8];
  uint8_t watch_count;
  uint64_t total_cycles;
  uint64_t total_frames;
} gbemu;

gbemu* gbemu_init(char* rom_name, const char* boot_rom, int test_category,
                  uint8_t disassemble_enable, const uint16_t* watch_addrs,
                  uint8_t watch_count);
int gbemu_step_frame(gbemu* gb);
void gbemu_free(gbemu* gb);
