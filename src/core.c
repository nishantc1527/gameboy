#include "gbemu/core.h"

#include <stdio.h>
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
#include "gbemu/util.h"

void disassemble(struct Cpu* cpu, struct Bus* bus, const uint16_t* watch_addrs,
                 uint8_t watch_count, uint64_t total_cycles);

struct gbemu* gbemu_init(char* rom_name, const char* boot_rom,
                         int test_category, uint8_t disassemble_enable,
                         const uint16_t* watch_addrs, uint8_t watch_count) {
  struct gbemu* gb = malloc(sizeof(struct gbemu));
  if (!gb) return NULL;
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
  gb->mmu = mmu_init(rom_name, boot_rom);
  if (!gb->mmu) {
    free(gb->cpu);
    free(gb);
    return NULL;
  }
  gb->apu = init_apu();
  gb->ppu = init_ppu();
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
    post_boot_cpu(gb->cpu, gb->cpu->cgb_mode, checksum);
    timer_post_boot(gb->timer);
  }
  return gb;
}

static void gbemu_free_components(struct gbemu* gb) {
  bus_free(gb->bus);
  serial_free(gb->serial);
  joypad_free(gb->joypad);
  dma_free(gb->dma);
  timer_free(gb->timer);
  free(gb->apu);
  free(gb->ppu);
  free(gb->cpu);
}

void gbemu_reset(struct gbemu* gb) {
  mmu_save(gb->mmu);
  gbemu_free_components(gb);
  mmu_free(gb->mmu);

  gb->cpu = init_cpu();
  gb->mmu = mmu_init(gb->rom_name, gb->boot_rom);
  gb->apu = init_apu();
  gb->ppu = init_ppu();
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
  gb->bdone = 0;
  gb->paused = 0;
  mmu_load(gb->mmu);
  if (mmu_boot_skipped(gb->mmu)) {
    uint8_t checksum = mmu_read_rom(gb->mmu, 0x014D);
    post_boot_cpu(gb->cpu, gb->cpu->cgb_mode, checksum);
    timer_post_boot(gb->timer);
  }
}

int gbemu_step_frame(struct gbemu* gb) {
  gb->ppu->frame = 0;
  while (!gb->ppu->frame) {
    uint8_t prev_lcdc_bit7 = bus_read(gb->bus, 0xFF40) & 0x80u;
    if (gb->disassemble_enable)
      disassemble(gb->cpu, gb->bus, gb->watch_addrs, gb->watch_count,
                  gb->total_cycles);
    const int cyc = cpu_step(gb->cpu, gb->bus);
    if (cyc == 0) return -1;
    if (gb->cpu->ldbb_fired) {
      gb->cpu->ldbb_fired = false;
      if (gb->test_category == TestAge || gb->test_category == TestMooneye ||
          gb->test_category == TestSame) {
        if (gb->cpu->B == 3 && gb->cpu->C == 5 && gb->cpu->D == 8 &&
            gb->cpu->E == 13 && gb->cpu->H == 21 && gb->cpu->L == 34)
          printf("Passed\n");
        else
          printf("Failed\n");
        gb->bdone = 1;
      } else if (gb->test_category == TestAcid2 ||
                 gb->test_category == TestMealybug) {
        gb->bdone = 1;
      }
    }
    if (gb->test_category == TestBlarggCpu ||
        gb->test_category == TestBlarggAudio ||
        gb->test_category == TestBlarggCpuTime ||
        gb->test_category == TestBlarggMemTime) {
      if (gb->serial->byte_ready)
        printf("%c", (char)serial_take_byte(gb->serial));
    }
    mmu_advance_rtc(gb->mmu, (uint64_t)cyc);
    gb->total_cycles += (uint64_t)cyc;
    gb->ppu->scn = (uint16_t)(gb->ppu->scn + cyc);
    if (gb->ppu->scn >= SCANLINE_LEN) {
      do_scanline(gb->ppu, gb->bus);
      gb->ppu->scn -= SCANLINE_LEN;
    }
    if (!prev_lcdc_bit7 && (bus_read(gb->bus, 0xFF40) & 0x80u)) {
      gb->ppu->scn = 4;
      bus_write(gb->bus, 0xFF44, 0);
    }
    update_lcd(gb->ppu, gb->bus);
    uint8_t cyc_left = (uint8_t)(cyc - gb->cpu->cyc_ext);
    gb->cpu->cyc_ext = 0;
    timer_tick(gb->timer, cyc_left, gb->apu, gb->cpu);
    upd_apu(gb->apu, gb->mmu, (uint8_t)cyc);
    int disp_cyc = check_interrupt(gb->cpu, gb->bus);
    if (disp_cyc > 0) {
      gb->total_cycles += (uint64_t)disp_cyc;
      gb->ppu->scn = (uint16_t)(gb->ppu->scn + disp_cyc);
      if (gb->ppu->scn >= SCANLINE_LEN) {
        do_scanline(gb->ppu, gb->bus);
        gb->ppu->scn -= SCANLINE_LEN;
      }
      update_lcd(gb->ppu, gb->bus);
      timer_tick(gb->timer, (uint8_t)disp_cyc, gb->apu, gb->cpu);
      upd_apu(gb->apu, gb->mmu, (uint8_t)disp_cyc);
    }
  }
  if ((gb->test_category == TestBlarggAudio ||
       gb->test_category == TestBlarggCgbSound) &&
      bus_read(gb->bus, 0xA001) == 0xDE && bus_read(gb->bus, 0xA002) == 0xB0 &&
      bus_read(gb->bus, 0xA003) == 0x61) {
    uint8_t status = bus_read(gb->bus, 0xA000);
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
    uint8_t result = bus_read(gb->bus, 0xFF82);
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
    gb->joypad->buttons[BTN_DOWN] = btn_down != 0;
    gb->joypad->buttons[BTN_A] = btn_a != 0;
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

void gbemu_free(struct gbemu* gb) {
  if (!gb) return;
  gbemu_free_components(gb);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}
