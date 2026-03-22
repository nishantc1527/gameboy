#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gbemu/bus.h"
#include "gbemu/core.h"
#include "gbemu/cpu.h"
#include "gbemu/joypad.h"
#include "gbemu/ppu.h"
#include "gbemu/serial.h"
#include "gbemu/util.h"
#include "rust.h"
#include "stb_image_write.h"

typedef struct TestState {
  int category;
  uint64_t frame_limit;
  uint8_t done;
} TestState;

static const uint8_t ACID_COLORS[5] = {0xFF, 0xAA, 0x55, 0x00, 0x00};

static int write_screenshot(struct Ppu* ppu, const char* path) {
  if (ppu->cgb_mode) {
    uint8_t buf[144][160][3];
    for (int y = 0; y < 144; y++)
      for (int x = 0; x < 160; x++) {
        uint16_t rgb555 = ppu->cgb_dsp[y][x];
        uint8_t r5 = rgb555 & 0x1F;
        uint8_t g5 = (rgb555 >> 5) & 0x1F;
        uint8_t b5 = (rgb555 >> 10) & 0x1F;
        buf[y][x][0] = (r5 << 3) | (r5 >> 2);
        buf[y][x][1] = (g5 << 3) | (g5 >> 2);
        buf[y][x][2] = (b5 << 3) | (b5 >> 2);
      }
    if (!stbi_write_png(path, 160, 144, 3, buf, 160 * 3)) {
      fprintf(stderr, "Failed to write screenshot: %s\n", path);
      return -1;
    }
  } else {
    uint8_t buf[144][160];
    for (int y = 0; y < 144; y++)
      for (int x = 0; x < 160; x++) buf[y][x] = ACID_COLORS[ppu->dsp[y][x]];
    if (!stbi_write_png(path, 160, 144, 1, buf, 160)) {
      fprintf(stderr, "Failed to write screenshot: %s\n", path);
      return -1;
    }
  }
  return 0;
}

static TestState test_init(int category) {
  TestState ts = {.category = category, .done = false};
  ts.frame_limit = (uint64_t)-1;
  return ts;
}

static void handle_test_frame(struct GBemu* gb, TestState* ts) {
  uint64_t f = gb->total_frames - BROM_FRAMES;
  int cat = ts->category;
  if (cat == TestBlarggCpu || cat == TestBlarggAudio ||
      cat == TestBlarggCpuTime || cat == TestBlarggMemTime ||
      cat == TestBlarggHaltBug || cat == TestBlarggInterruptTime ||
      cat == TestBlarggMemTime2 || cat == TestBlarggOamBug ||
      cat == TestBlarggCgbSound) {
    while (serial_has_byte(gb->serial))
      printf("%c", (char)serial_take_byte(gb->serial));
  }
  if (gb->cpu->ldbb_fired) {
    gb->cpu->ldbb_fired = false;
    if (cat == TestAge || cat == TestMooneye || cat == TestSame) {
      if (gb->cpu->B == 3 && gb->cpu->C == 5 && gb->cpu->D == 8 &&
          gb->cpu->E == 13 && gb->cpu->H == 21 && gb->cpu->L == 34)
        printf("Passed\n");
      else
        printf("Failed\n");
      ts->done = true;
    } else if (cat == TestAcid2 || cat == TestMealybug)
      ts->done = true;
  }
  if (cat == TestBlarggAudio || cat == TestBlarggCgbSound) {
    if (bus_read(gb->bus, 0xA001) == 0xDE &&
        bus_read(gb->bus, 0xA002) == 0xB0 &&
        bus_read(gb->bus, 0xA003) == 0x61) {
      uint8_t status = bus_read(gb->bus, 0xA000);
      if (status != 0x80) {
        if (status == 0x00)
          printf("Passed\n");
        else
          printf("Failed %d\n", status);
        ts->done = true;
      }
    }
  }
  if (cat == TestMicro && gb->total_frames >= BROM_FRAMES + 10) {
    uint8_t result = bus_read(gb->bus, 0xFF82);
    if (result == 0x01)
      printf("Passed\n");
    else if (result == 0xFF)
      printf("Failed\n");
    else
      printf("TEST DID NOT COMPLETE\n");
    ts->done = true;
  }
  if (cat == TestRtc3Basic || cat == TestRtc3Range || cat == TestRtc3Sub) {
    int btn_down = 0, btn_a = 0;
    if (cat == TestRtc3Basic) {
      btn_a = (f >= 12 && f < 22) ? 1 : 0;
    } else if (cat == TestRtc3Range) {
      btn_down = (f >= 12 && f < 22) ? 1 : 0;
      btn_a = (f >= 32 && f < 42) ? 1 : 0;
    } else {
      btn_down = ((f >= 12 && f < 22) || (f >= 32 && f < 42)) ? 1 : 0;
      btn_a = (f >= 52 && f < 62) ? 1 : 0;
    }
    gb->joypad->buttons[BTN_DOWN] = btn_down != 0;
    gb->joypad->buttons[BTN_A] = btn_a != 0;
    if ((cat == TestRtc3Basic &&
         gb->total_frames >= BROM_FRAMES + 22 + 13 * 60) ||
        (cat == TestRtc3Range &&
         gb->total_frames >= BROM_FRAMES + 42 + 8 * 60) ||
        (cat == TestRtc3Sub && gb->total_frames >= BROM_FRAMES + 62 + 26 * 60))
      ts->done = true;
  }
  if (cat == TestBlarggCpu && gb->total_frames >= BROM_FRAMES + 60 * 60)
    ts->done = true;
  if (cat == TestBlarggAudio && gb->total_frames >= BROM_FRAMES + 60 * 60)
    ts->done = true;
  if ((cat == TestBlarggCpuTime || cat == TestBlarggMemTime ||
       cat == TestBlarggHaltBug || cat == TestBlarggInterruptTime) &&
      gb->total_frames >= BROM_FRAMES + 120)
    ts->done = true;
  if (cat == TestBlarggMemTime2 && gb->total_frames >= BROM_FRAMES + 240)
    ts->done = true;
  if (cat == TestBlarggOamBug && gb->total_frames >= BROM_FRAMES + 1260)
    ts->done = true;
  if (cat == TestBlarggCgbSound && gb->total_frames >= BROM_FRAMES + 2220)
    ts->done = true;
  if (cat == TestBully && gb->total_frames >= BROM_FRAMES + 30) ts->done = true;
  if (cat == TestGambatte && gb->total_frames >= BROM_FRAMES + 15)
    ts->done = true;
  if (cat == TestLittle && gb->total_frames >= BROM_FRAMES + 30)
    ts->done = true;
  if (cat == TestMbc3 && gb->total_frames >= BROM_FRAMES + 60) ts->done = true;
  if (cat == TestScribble && gb->total_frames >= BROM_FRAMES + 10)
    ts->done = true;
  if (cat == TestStrike && gb->total_frames >= BROM_FRAMES + 30)
    ts->done = true;
  if (cat == TestTurtle && gb->total_frames >= BROM_FRAMES + 30)
    ts->done = true;
}

int main(int argc, char* argv[]) {
  char* rom_name = NULL;
  char* boot_rom = "";
  char* screenshot_path = NULL;
  int test_category = -1;
  int disassemble_enable = 0;
  uint16_t watch_addrs[8];
  uint8_t watch_count = 0;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--version")) {
      printf("gbemu " GBEMU_VERSION "\n");
      return 0;
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      printf("Usage: gbemu_headless -r <rom.gb> [options]\n\n");
      printf("Options:\n");
      printf("  -r, --rom <file>        ROM file to run\n");
      printf("  -b, --boot-rom <file>   Boot ROM file (default: boot.rom)\n");
      printf("  -t, --test <category>   Test category for automated testing\n");
      printf("  -s, --screenshot <file> Save screenshot to PNG file\n");
      printf("  -d, --disassembly       Print per-instruction disassembly\n");
      printf("  -w, --watch <hex addr>  Watch memory address (up to 8)\n");
      printf("      --version           Print version and exit\n");
      printf("  -h, --help              Show this help\n\n");
      printf(
          "Test categories: blargg_cpu, blargg_cpu_time, blargg_mem_time,\n");
      printf("  blargg_audio, blargg_halt_bug, blargg_interrupt_time,\n");
      printf("  blargg_mem_time2, blargg_oam_bug, blargg_cgb_sound,\n");
      printf("  acid2, mbc3, rtc3_basic, rtc3_range, rtc3_sub, gambatte,\n");
      printf("  micro, little, mealybug, mooneye, same, age, bully,\n");
      printf("  scribble, strike, turtle\n");
      return 0;
    } else if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) &&
               i + 1 < argc)
      rom_name = argv[++i];
    else if ((!strcmp(argv[i], "-b") || !strcmp(argv[i], "--boot-rom")) &&
             i + 1 < argc)
      boot_rom = argv[++i];
    else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) &&
             i + 1 < argc) {
      char* s = argv[++i];
      if (!strcmp(s, "age"))
        test_category = TestAge;
      else if (!strcmp(s, "blargg_cpu"))
        test_category = TestBlarggCpu;
      else if (!strcmp(s, "blargg_audio"))
        test_category = TestBlarggAudio;
      else if (!strcmp(s, "blargg_cpu_time"))
        test_category = TestBlarggCpuTime;
      else if (!strcmp(s, "blargg_mem_time"))
        test_category = TestBlarggMemTime;
      else if (!strcmp(s, "blargg_halt_bug"))
        test_category = TestBlarggHaltBug;
      else if (!strcmp(s, "blargg_interrupt_time"))
        test_category = TestBlarggInterruptTime;
      else if (!strcmp(s, "blargg_mem_time2"))
        test_category = TestBlarggMemTime2;
      else if (!strcmp(s, "blargg_oam_bug"))
        test_category = TestBlarggOamBug;
      else if (!strcmp(s, "blargg_cgb_sound"))
        test_category = TestBlarggCgbSound;
      else if (!strcmp(s, "bully"))
        test_category = TestBully;
      else if (!strcmp(s, "acid2"))
        test_category = TestAcid2;
      else if (!strcmp(s, "gambatte"))
        test_category = TestGambatte;
      else if (!strcmp(s, "micro"))
        test_category = TestMicro;
      else if (!strcmp(s, "little"))
        test_category = TestLittle;
      else if (!strcmp(s, "mbc3"))
        test_category = TestMbc3;
      else if (!strcmp(s, "mealybug"))
        test_category = TestMealybug;
      else if (!strcmp(s, "mooneye"))
        test_category = TestMooneye;
      else if (!strcmp(s, "same"))
        test_category = TestSame;
      else if (!strcmp(s, "rtc3_basic"))
        test_category = TestRtc3Basic;
      else if (!strcmp(s, "rtc3_range"))
        test_category = TestRtc3Range;
      else if (!strcmp(s, "rtc3_sub"))
        test_category = TestRtc3Sub;
      else if (!strcmp(s, "scribble"))
        test_category = TestScribble;
      else if (!strcmp(s, "strike"))
        test_category = TestStrike;
      else if (!strcmp(s, "turtle"))
        test_category = TestTurtle;
      else {
        fprintf(stderr, "UNKNOWN TEST CATEGORY: %s\n", s);
        return 1;
      }
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble_enable = 1;
    else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--screenshot")) &&
             i + 1 < argc)
      screenshot_path = argv[++i];
    else if ((!strcmp(argv[i], "-w") || !strcmp(argv[i], "--watch")) &&
             i + 1 < argc) {
      if (watch_count < 8)
        watch_addrs[watch_count++] = (uint16_t)strtol(argv[++i], NULL, 16);
      else
        i++;
    } else {
      fprintf(stderr, "UNKNOWN COMMAND LINE OPTION: %s\n", argv[i]);
      return 1;
    }
  }
  if (!rom_name) {
    fprintf(stderr, "MUST PROVIDE ROM FILE\n");
    return 1;
  }
  struct GBemu* gb = gbemu_init(rom_name, boot_rom, (uint8_t)disassemble_enable,
                                watch_addrs, watch_count);
  if (!gb) return 1;
  TestState ts = test_init(test_category);
  while (!ts.done) {
    if (gbemu_step_frame(gb) == -1) {
      gbemu_free(gb);
      return 1;
    }
    handle_test_frame(gb, &ts);
  }
  if (screenshot_path && write_screenshot(gb->ppu, screenshot_path)) return 1;
  gbemu_free(gb);
  return 0;
}
