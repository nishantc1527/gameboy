#include "gbemu/core.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/boot_roms.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/dma.h"
#include "gbemu/joypad.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/serial.h"
#include "gbemu/timer.h"

void disassemble(struct Cpu* cpu, struct Bus* bus, const uint16_t* watch_addrs,
                 uint8_t watch_count, uint64_t total_cycles);

static void init_components(struct GBemu* gb) {
  bool cgb_mode = mmu_is_cgb(gb->mmu);
  bool cgb_compat = mmu_is_cgb_compat(gb->mmu);
  gb->cpu = cpu_init(cgb_mode);
  gb->apu = apu_init();
  gb->ppu = ppu_init(cgb_mode, cgb_compat);
  gb->timer = timer_init();
  gb->dma = dma_init();
  gb->joypad = joypad_init();
  gb->serial = serial_init();
  gb->bus = bus_init(gb->mmu, gb->cpu, gb->ppu, gb->apu, gb->timer, gb->dma,
                     gb->joypad, gb->serial);
  gb->total_cycles = 0;
  gb->total_frames = 0;
  gb->is_paused = false;
  mmu_load(gb->mmu);
}

static void system_tick(struct GBemu* gb, uint8_t cycles) {
  ppu_tick(gb->ppu, gb->bus, cycles);
  if (dma_hdma_block_pending(gb->dma)) {
    dma_clear_hdma_block_pending(gb->dma);
    dma_hdma_block(gb->dma, gb->bus);
  }
  dma_tick(gb->dma, gb->bus, cycles);
  uint8_t timer_remaining =
      (uint8_t)(cycles - timer_consume_sub_cycles(gb->timer));
  if (timer_remaining) {
    timer_tick(gb->timer, timer_remaining, gb->bus);
  }
  apu_tick(gb->apu, cycles);
  mmu_advance_rtc(gb->mmu, (uint64_t)cycles);
  gb->total_cycles += (uint64_t)cycles;
}

struct GBemu* gbemu_init(char* rom_name, bool disassemble_enable,
                         const uint16_t* watch_addrs, uint8_t watch_count) {
  struct GBemu* gb = malloc(sizeof(struct GBemu));
  if (!gb) {
    return NULL;
  }
  gb->rom_name = rom_name;
  gb->disassemble_enable = disassemble_enable;
  gb->is_paused = false;
  gb->fast_forward = false;
  if (watch_addrs) {
    gb->watch_count = watch_count < 8 ? watch_count : 8;
  } else {
    gb->watch_count = 0;
  }
  for (uint8_t i = 0; i < gb->watch_count; i++) {
    gb->watch_addrs[i] = watch_addrs[i];
  }
  gb->mmu = mmu_init(rom_name, dmg_boot_rom, sizeof(dmg_boot_rom), cgb_boot_rom,
                     sizeof(cgb_boot_rom));
  if (!gb->mmu) {
    free(gb);
    return NULL;
  }
  init_components(gb);
  return gb;
}

static void gbemu_free_components(struct GBemu* gb) {
  bus_free(gb->bus);
  serial_free(gb->serial);
  joypad_free(gb->joypad);
  dma_free(gb->dma);
  timer_free(gb->timer);
  apu_free(gb->apu);
  free(gb->ppu);
  free(gb->cpu);
}

void gbemu_reset(struct GBemu* gb) {
  mmu_save(gb->mmu);
  gbemu_free_components(gb);
  mmu_free(gb->mmu);

  gb->mmu = mmu_init(gb->rom_name, dmg_boot_rom, sizeof(dmg_boot_rom),
                     cgb_boot_rom, sizeof(cgb_boot_rom));
  init_components(gb);
}

int gbemu_step_frame(struct GBemu* gb) {
  ppu_begin_frame(gb->ppu);
  while (!ppu_frame_ready(gb->ppu)) {
    if (gb->disassemble_enable) {
      disassemble(gb->cpu, gb->bus, gb->watch_addrs, gb->watch_count,
                  gb->total_cycles);
    }
    uint8_t cyc = cpu_step(gb->cpu, gb->bus);
    if (cyc == 0) {
      return -1;
    }
    system_tick(gb, cyc);
    uint8_t dispatch_cyc = cpu_check_interrupts(gb->cpu, gb->bus);
    if (dispatch_cyc) {
      system_tick(gb, dispatch_cyc);
    }
  }
  gb->total_frames++;
  return 0;
}

void gbemu_free(struct GBemu* gb) {
  if (!gb) {
    return;
  }
  gbemu_free_components(gb);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}
