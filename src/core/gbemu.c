#include "gbemu/gbemu.h"

#include <stdio.h>
#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "rom_locs.h"

gbemu* gbemu_init(char* rom_name, int test_category, uint8_t disassemble_enable,
                  const uint16_t* watch_addrs, uint8_t watch_count) {
  gbemu* gb = malloc(sizeof(gbemu));
  gb->rom_name = rom_name;
  gb->test_category = test_category;
  gb->disassemble_enable = disassemble_enable;
  gb->bdone = 0;
  gb->watch_count = watch_addrs ? (watch_count < 8 ? watch_count : 8) : 0;
  for (uint8_t i = 0; i < gb->watch_count; i++)
    gb->watch_addrs[i] = watch_addrs[i];
  gb->cpu = init_cpu();
  gb->mmu = mmu_init(rom_name, BOOT_ROM_FILE, (int8_t)test_category);
  gb->apu = init_apu();
  gb->ppu = init_ppu();
  gb->total_cycles = 0;
  gb->total_frames = 0;
  mmu_load(gb->mmu);
  return gb;
}

int gbemu_step_frame(gbemu* gb) {
  gb->ppu->frame = 0;
  while (!gb->ppu->frame) {
    const int cyc = step(gb->cpu, gb->mmu, gb->apu, gb->disassemble_enable,
                         gb->test_category, &gb->bdone, gb->watch_addrs,
                         gb->watch_count, gb->total_cycles);
    if (cyc == -1) return -1;
    gb->total_cycles += (uint64_t)cyc;
    gb->ppu->scn = (uint16_t)(gb->ppu->scn + cyc);
    if (gb->ppu->scn >= SCANLINE_LEN) {
      do_scanline(gb->ppu, gb->mmu);
      gb->ppu->scn -= SCANLINE_LEN;
    }
    update_lcd(gb->ppu, gb->mmu);
    uint8_t cyc_left = (uint8_t)(cyc - gb->cpu->cyc_ext);
    gb->cpu->cyc_ext = 0;
    update_timer(gb->cpu, gb->mmu, gb->apu, cyc_left);
    check_dma(gb->mmu);
    upd_apu(gb->apu, gb->mmu, cyc_left);
    check_interrupt(gb->cpu, gb->mmu);
  }
  if (gb->test_category == TestBlarggAudio &&
      mmu_r_mem(gb->mmu, 0xA001) == 0xDE &&
      mmu_r_mem(gb->mmu, 0xA002) == 0xB0 &&
      mmu_r_mem(gb->mmu, 0xA003) == 0x61) {
    uint8_t status = mmu_r_mem(gb->mmu, 0xA000);
    if (status != 0x80) {
      if (status == 0x00)
        printf("Passed\n");
      else
        printf("Failed %d\n", status);
      gb->bdone = 1;
    }
  }
  gb->total_frames++;
  if ((gb->test_category == TestBlarggCpu ||
       gb->test_category == TestBlarggAudio) &&
      gb->total_frames >= BROM_FRAMES + 60 * 60)
    gb->bdone = 1;
  if ((gb->test_category == TestBlarggCpuTime ||
       gb->test_category == TestBlarggMemTime) &&
      gb->total_frames >= BROM_FRAMES + 120)
    gb->bdone = 1;
  if (gb->test_category == TestBully && gb->total_frames >= BROM_FRAMES + 30)
    gb->bdone = 1;
  if (gb->test_category == TestGambatte && gb->total_frames >= BROM_FRAMES + 15)
    gb->bdone = 1;
  if (gb->test_category == TestMicro && gb->total_frames >= BROM_FRAMES + 10) {
    uint8_t result = mmu_r_mem(gb->mmu, 0xFF82);
    if (result == 0x01)
      printf("Passed\n");
    else if (result == 0xFF)
      printf("Failed\n");
    else
      printf("TEST DID NOT COMPLETE\n");
    gb->bdone = 1;
  }
  if (gb->test_category == TestLittle && gb->total_frames >= BROM_FRAMES + 30)
    gb->bdone = 1;
  if (gb->test_category == TestMbc3 && gb->total_frames >= BROM_FRAMES + 60)
    gb->bdone = 1;
  if (gb->test_category == TestScribble && gb->total_frames >= BROM_FRAMES + 10)
    gb->bdone = 1;
  if (gb->test_category == TestStrike && gb->total_frames >= BROM_FRAMES + 30)
    gb->bdone = 1;
  if (gb->test_category == TestTurtle && gb->total_frames >= BROM_FRAMES + 30)
    gb->bdone = 1;
  return 0;
}

void gbemu_free(gbemu* gb) {
  if (!gb) return;
  free(gb->cpu);
  free(gb->ppu);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}
