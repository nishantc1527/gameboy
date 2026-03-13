#include "gbemu/gbemu.h"

#include <stdio.h>
#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"

gbemu* gbemu_init(char* rom_name, const char* boot_rom, int test_category,
                  uint8_t disassemble_enable, const uint16_t* watch_addrs,
                  uint8_t watch_count) {
  gbemu* gb = malloc(sizeof(gbemu));
  gb->rom_name = rom_name;
  gb->boot_rom = boot_rom;
  gb->test_category = test_category;
  gb->disassemble_enable = disassemble_enable;
  gb->bdone = 0;
  gb->paused = 0;
  gb->fast_forward = 0;
  gb->watch_count = watch_addrs ? (watch_count < 8 ? watch_count : 8) : 0;
  for (uint8_t i = 0; i < gb->watch_count; i++)
    gb->watch_addrs[i] = watch_addrs[i];
  gb->cpu = init_cpu();
  gb->mmu = mmu_init(rom_name, boot_rom, (int8_t)test_category);
  if (!gb->mmu) {
    free(gb->cpu);
    free(gb);
    return NULL;
  }
  gb->apu = init_apu();
  gb->ppu = init_ppu();
  gb->cpu->cgb_mode = mmu_is_cgb(gb->mmu) ? 1 : 0;
  gb->ppu->cgb_mode = gb->cpu->cgb_mode;
  gb->total_cycles = 0;
  gb->total_frames = 0;
  mmu_load(gb->mmu);
  if (mmu_boot_skipped(gb->mmu)) {
    uint8_t checksum = mmu_r_mem(gb->mmu, 0x014D);
    post_boot_cpu(gb->cpu, gb->cpu->cgb_mode, checksum);
  }
  return gb;
}

void gbemu_reset(gbemu* gb) {
  free(gb->cpu);
  free(gb->ppu);
  free(gb->apu);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  gb->cpu = init_cpu();
  gb->mmu = mmu_init(gb->rom_name, gb->boot_rom, (int8_t)gb->test_category);
  gb->apu = init_apu();
  gb->ppu = init_ppu();
  gb->cpu->cgb_mode = mmu_is_cgb(gb->mmu) ? 1 : 0;
  gb->ppu->cgb_mode = gb->cpu->cgb_mode;
  gb->total_cycles = 0;
  gb->total_frames = 0;
  gb->bdone = 0;
  gb->paused = 0;
  mmu_load(gb->mmu);
  if (mmu_boot_skipped(gb->mmu)) {
    uint8_t checksum = mmu_r_mem(gb->mmu, 0x014D);
    post_boot_cpu(gb->cpu, gb->cpu->cgb_mode, checksum);
  }
}

int gbemu_step_frame(gbemu* gb) {
  gb->ppu->frame = 0;
  while (!gb->ppu->frame) {
    uint8_t prev_lcdc_bit7 = mmu_r_mem(gb->mmu, 0xFF40) & 0x80;
    const int cyc = step(gb->cpu, gb->mmu, gb->apu, gb->disassemble_enable,
                         gb->test_category, &gb->bdone, gb->watch_addrs,
                         gb->watch_count, gb->total_cycles);
    if (cyc == -1) return -1;
    mmu_advance_rtc(gb->mmu, (uint64_t)cyc);
    gb->total_cycles += (uint64_t)cyc;
    gb->ppu->scn = (uint16_t)(gb->ppu->scn + cyc);
    if (gb->ppu->scn >= SCANLINE_LEN) {
      do_scanline(gb->ppu, gb->mmu);
      gb->ppu->scn -= SCANLINE_LEN;
    }
    if (!prev_lcdc_bit7 && (mmu_r_mem(gb->mmu, 0xFF40) & 0x80)) {
      gb->ppu->scn = 4;
      mmu_w_mem(gb->mmu, 0xFF44, 0);
    }
    update_lcd(gb->ppu, gb->mmu);
    uint8_t cyc_left = (uint8_t)(cyc - gb->cpu->cyc_ext);
    gb->cpu->cyc_ext = 0;
    update_timer(gb->cpu, gb->mmu, gb->apu, cyc_left);
    check_dma(gb->mmu);
    upd_apu(gb->apu, gb->mmu, (uint8_t)cyc);
    int disp_cyc = check_interrupt(gb->cpu, gb->mmu);
    if (disp_cyc > 0) {
      gb->total_cycles += (uint64_t)disp_cyc;
      gb->ppu->scn = (uint16_t)(gb->ppu->scn + disp_cyc);
      if (gb->ppu->scn >= SCANLINE_LEN) {
        do_scanline(gb->ppu, gb->mmu);
        gb->ppu->scn -= SCANLINE_LEN;
      }
      update_lcd(gb->ppu, gb->mmu);
      update_timer(gb->cpu, gb->mmu, gb->apu, (uint8_t)disp_cyc);
      upd_apu(gb->apu, gb->mmu, (uint8_t)disp_cyc);
    }
  }
  if ((gb->test_category == TestBlarggAudio ||
       gb->test_category == TestBlarggCgbSound) &&
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
  if ((gb->test_category == TestBlarggHaltBug ||
       gb->test_category == TestBlarggInterruptTime) &&
      gb->total_frames >= BROM_FRAMES + 120)
    gb->bdone = 1;
  if (gb->test_category == TestBlarggMemTime2 &&
      gb->total_frames >= BROM_FRAMES + 240)
    gb->bdone = 1;
  if (gb->test_category == TestBlarggOamBug &&
      gb->total_frames >= BROM_FRAMES + 1260)
    gb->bdone = 1;
  if (gb->test_category == TestBlarggCgbSound &&
      gb->total_frames >= BROM_FRAMES + 2220)
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
  if (gb->test_category == TestRtc3Basic ||
      gb->test_category == TestRtc3Range || gb->test_category == TestRtc3Sub) {
    uint64_t f = gb->total_frames - BROM_FRAMES;
    int btn_down = 0, btn_a = 0;
    if (gb->test_category == TestRtc3Basic) {
      btn_a = (f >= 12 && f < 22) ? 1 : 0;
    } else if (gb->test_category == TestRtc3Range) {
      btn_down = (f >= 12 && f < 22) ? 1 : 0;
      btn_a = (f >= 32 && f < 42) ? 1 : 0;
    } else {
      btn_down = ((f >= 12 && f < 22) || (f >= 32 && f < 42)) ? 1 : 0;
      btn_a = (f >= 52 && f < 62) ? 1 : 0;
    }
    gb->ppu->in[BTN_DOWN] = btn_down;
    gb->ppu->in[BTN_A] = btn_a;
    update_input(gb->ppu, gb->mmu);
    if ((gb->test_category == TestRtc3Basic &&
         gb->total_frames >= BROM_FRAMES + 22 + 13 * 60) ||
        (gb->test_category == TestRtc3Range &&
         gb->total_frames >= BROM_FRAMES + 42 + 8 * 60) ||
        (gb->test_category == TestRtc3Sub &&
         gb->total_frames >= BROM_FRAMES + 62 + 26 * 60))
      gb->bdone = 1;
  }
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
