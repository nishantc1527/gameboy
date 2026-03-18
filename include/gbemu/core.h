#pragma once

#include <stdint.h>

#define GBEMU_VERSION "1.0.0"

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
  const char* boot_rom;
  int test_category;
  uint8_t disassemble_enable;
  uint8_t bdone;
  uint8_t paused;
  uint8_t fast_forward;
  uint16_t watch_addrs[8];
  uint8_t watch_count;
  uint64_t total_cycles;
  uint64_t total_frames;
} gbemu;

typedef struct {
  int scale;
  uint32_t dmg_palette[4];
  bool fullscreen;
  float volume;
  char key_a[64];
  char key_b[64];
  char key_start[64];
  char key_select[64];
  char key_up[64];
  char key_down[64];
  char key_left[64];
  char key_right[64];
  char key_pause[64];
  char key_screenshot[64];
  char boot_rom[512];
  char last_rom[512];
  char recent_roms[10][512];
  int recent_rom_count;
} Settings;

extern Settings g_settings;

gbemu* gbemu_init(char* rom_name, const char* boot_rom, int test_category,
                  uint8_t disassemble_enable, const uint16_t* watch_addrs,
                  uint8_t watch_count);
int gbemu_step_frame(gbemu* gb);
void gbemu_free(gbemu* gb);
void gbemu_reset(gbemu* gb);

void settings_defaults(Settings* s);
void settings_load(Settings* s);
void settings_save(const Settings* s);
void settings_add_recent_rom(Settings* s, const char* path);
