#include "gbemu/core.h"

#include <stdlib.h>

#include "gbemu/apu.h"
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

static void system_tick(struct GBemu* gb, uint8_t cycles) {
  ppu_tick(gb->ppu, gb->bus, cycles);
  dma_tick(gb->dma, gb->bus, cycles);
  uint8_t timer_remaining = (uint8_t)(cycles - gb->timer->sub_instr_cycles);
  gb->timer->sub_instr_cycles = 0;
  if (timer_remaining) timer_tick(gb->timer, timer_remaining, gb->apu, gb->cpu);
  apu_tick(gb->apu, (uint8_t)cycles);
  mmu_advance_rtc(gb->mmu, (uint64_t)cycles);
  gb->total_cycles += (uint64_t)cycles;
}

struct GBemu* gbemu_init(char* rom_name, const char* boot_rom,
                         uint8_t disassemble_enable,
                         const uint16_t* watch_addrs, uint8_t watch_count) {
  struct GBemu* gb = malloc(sizeof(struct GBemu));
  if (!gb) return NULL;
  gb->rom_name = rom_name;
  gb->boot_rom = boot_rom;
  gb->disassemble_enable = disassemble_enable;
  gb->paused = 0;
  gb->fast_forward = 0;
  gb->watch_count = watch_addrs ? (watch_count < 8 ? watch_count : 8) : 0;
  for (uint8_t i = 0; i < gb->watch_count; i++)
    gb->watch_addrs[i] = watch_addrs[i];

  gb->cpu = cpu_init();
  gb->mmu = mmu_init(rom_name, boot_rom);
  if (!gb->mmu) {
    free(gb->cpu);
    free(gb);
    return NULL;
  }
  gb->apu = apu_init();
  gb->ppu = ppu_init();
  gb->timer = timer_init();
  gb->dma = dma_init();
  gb->joypad = joypad_init();
  gb->serial = serial_init();
  gb->bus = bus_init(gb->mmu, gb->cpu, gb->ppu, gb->apu, gb->timer, gb->dma,
                     gb->joypad, gb->serial);

  gb->cpu->cgb_mode = mmu_is_cgb(gb->mmu) ? 1 : 0;
  gb->ppu->cgb_mode = gb->cpu->cgb_mode;
  gb->ppu->cgb_compat = mmu_is_cgb_compat(gb->mmu) ? 1 : 0;
  gb->total_cycles = 0;
  gb->total_frames = 0;

  mmu_load(gb->mmu);
  if (mmu_boot_skipped(gb->mmu)) {
    uint8_t checksum = mmu_read_rom(gb->mmu, 0x014D);
    cpu_post_boot(gb->cpu, gb->cpu->cgb_mode, checksum);
    timer_post_boot(gb->timer);
    ppu_post_boot(gb->ppu, gb->cpu->cgb_mode != 0);
  }
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

  gb->cpu = cpu_init();
  gb->mmu = mmu_init(gb->rom_name, gb->boot_rom);
  gb->apu = apu_init();
  gb->ppu = ppu_init();
  gb->timer = timer_init();
  gb->dma = dma_init();
  gb->joypad = joypad_init();
  gb->serial = serial_init();
  gb->bus = bus_init(gb->mmu, gb->cpu, gb->ppu, gb->apu, gb->timer, gb->dma,
                     gb->joypad, gb->serial);

  gb->cpu->cgb_mode = mmu_is_cgb(gb->mmu) ? 1 : 0;
  gb->ppu->cgb_mode = gb->cpu->cgb_mode;
  gb->ppu->cgb_compat = mmu_is_cgb_compat(gb->mmu) ? 1 : 0;
  gb->total_cycles = 0;
  gb->total_frames = 0;
  gb->paused = 0;
  mmu_load(gb->mmu);
  if (mmu_boot_skipped(gb->mmu)) {
    uint8_t checksum = mmu_read_rom(gb->mmu, 0x014D);
    cpu_post_boot(gb->cpu, gb->cpu->cgb_mode, checksum);
    timer_post_boot(gb->timer);
    ppu_post_boot(gb->ppu, gb->cpu->cgb_mode != 0);
  }
}

int gbemu_step_frame(struct GBemu* gb) {
  gb->ppu->frame_ready = 0;
  while (!gb->ppu->frame_ready) {
    if (gb->disassemble_enable)
      disassemble(gb->cpu, gb->bus, gb->watch_addrs, gb->watch_count,
                  gb->total_cycles);
    uint8_t cyc = cpu_step(gb->cpu, gb->bus);
    if (cyc == 0) return -1;
    system_tick(gb, cyc);
    uint8_t dispatch_cyc = cpu_check_interrupts(gb->cpu, gb->bus);
    if (dispatch_cyc) system_tick(gb, dispatch_cyc);
  }
  gb->total_frames++;
  return 0;
}

void gbemu_free(struct GBemu* gb) {
  if (!gb) return;
  gbemu_free_components(gb);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}
