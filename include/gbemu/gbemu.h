#pragma once

#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"

#define TEST_BLARGG 0
#define TEST_MOONEYE 1

typedef struct {
  struct CPU* cpu;
  Mmu* mmu;
  struct PPU* ppu;
  char* rom_name;
  int test_category;
  uint8_t headless;
  uint8_t disassemble_enable;
  uint8_t b_done;
} GbEmu;

GbEmu* gbemu_init(char* rom_name, int test_category, uint8_t headless,
                  uint8_t disassemble_enable);
int gbemu_run_frame(GbEmu* gb);
void gbemu_free(GbEmu* gb);
