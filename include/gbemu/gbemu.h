#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"

#define TEST_BLARGG 0
#define TEST_MOONEYE 1
#define TEST_BLARGG_AUDIO 2
#define TEST_ACID2 3

typedef struct {
  struct Apu* apu;
  struct Cpu* cpu;
  Mmu* mmu;
  struct Ppu* ppu;
  char* rom_name;
  int test_category;
  uint8_t disassemble_enable;
  uint8_t bdone;
} gbemu;

gbemu* gbemu_init(char* rom_name, int test_category,
                  uint8_t disassemble_enable);
int gbemu_step_frame(gbemu* gb);
void gbemu_free(gbemu* gb);
