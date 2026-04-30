#include <stdint.h>
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

struct TestState {
  int category;
  uint64_t frame_limit;
  bool done;
};

static const uint8_t ACID_COLORS[5] = {0xFF, 0xAA, 0x55, 0x00, 0x00};

static int write_screenshot(struct Ppu* ppu, const char* path) {
  if (ppu_get_cgb_mode(ppu)) {
    const uint16_t* cgb_fb = ppu_get_cgb_framebuffer(ppu);
    uint8_t buf[SCRN_HEIGHT][SCRN_WIDTH][3];
    for (int row = 0; row < SCRN_HEIGHT; row++) {
      for (int col = 0; col < SCRN_WIDTH; col++) {
        uint16_t rgb555 = cgb_fb[(row * SCRN_WIDTH) + col];
        uint8_t red5 = (uint8_t)(((unsigned)rgb555 >> 0U) & 0x1FU);
        uint8_t grn5 = (uint8_t)(((unsigned)rgb555 >> 5U) & 0x1FU);
        uint8_t blu5 = (uint8_t)(((unsigned)rgb555 >> 10U) & 0x1FU);
        buf[row][col][0] =
            (uint8_t)(((unsigned)red5 << 3U) | ((unsigned)red5 >> 2U));
        buf[row][col][1] =
            (uint8_t)(((unsigned)grn5 << 3U) | ((unsigned)grn5 >> 2U));
        buf[row][col][2] =
            (uint8_t)(((unsigned)blu5 << 3U) | ((unsigned)blu5 >> 2U));
      }
    }
    if (!stbi_write_png(path, SCRN_WIDTH, SCRN_HEIGHT, 3, buf,
                        SCRN_WIDTH * 3)) {
      (void)fprintf(stderr, "Failed to write screenshot: %s\n", path);
      return -1;
    }
  } else {
    const uint8_t* dmg_fb = ppu_get_dmg_framebuffer(ppu);
    uint8_t buf[SCRN_HEIGHT][SCRN_WIDTH];
    for (int row = 0; row < SCRN_HEIGHT; row++) {
      for (int col = 0; col < SCRN_WIDTH; col++) {
        buf[row][col] = ACID_COLORS[dmg_fb[(row * SCRN_WIDTH) + col]];
      }
    }
    if (!stbi_write_png(path, SCRN_WIDTH, SCRN_HEIGHT, 1, buf, SCRN_WIDTH)) {
      (void)fprintf(stderr, "Failed to write screenshot: %s\n", path);
      return -1;
    }
  }
  return 0;
}

static struct TestState test_init(int category) {
  struct TestState ts = {.category = category, .done = false};
  ts.frame_limit = (uint64_t)-1;
  return ts;
}

static void handle_test_frame(struct GBemu* gb, struct TestState* ts) {
  uint64_t f = gb->total_frames - BROM_FRAMES;
  int cat = ts->category;
  if (cat == TestBlarggCpu || cat == TestBlarggAudio ||
      cat == TestBlarggCpuTime || cat == TestBlarggMemTime ||
      cat == TestBlarggHaltBug || cat == TestBlarggInterruptTime ||
      cat == TestBlarggMemTime2 || cat == TestBlarggOamBug ||
      cat == TestBlarggCgbSound) {
    while (serial_has_byte(gb->serial)) {
      printf("%c", (char)serial_take_byte(gb->serial));
    }
  }
  if (cpu_check_ld_b_b(gb->cpu)) {
    if (cat == TestAge || cat == TestMooneye || cat == TestSame) {
      if (cpu_get_b(gb->cpu) == 3 && cpu_get_c(gb->cpu) == 5 &&
          cpu_get_d(gb->cpu) == 8 && cpu_get_e(gb->cpu) == 13 &&
          cpu_get_h(gb->cpu) == 21 && cpu_get_l(gb->cpu) == 34) {
        printf("Passed\n");
      } else {
        printf("Failed\n");
      }
      ts->done = true;
    } else if (cat == TestAcid2 || cat == TestMealybug) {
      {
        ts->done = true;
      }
    }
  }
  if (cat == TestBlarggAudio || cat == TestBlarggCgbSound) {
    if (bus_read(gb->bus, 0xA001) == 0xDE &&
        bus_read(gb->bus, 0xA002) == 0xB0 &&
        bus_read(gb->bus, 0xA003) == 0x61) {
      uint8_t status = bus_read(gb->bus, 0xA000);
      if (status != 0x80) {
        if (status == 0x00) {
          printf("Passed\n");
        } else {
          printf("Failed %d\n", status);
        }
        ts->done = true;
      }
    }
  }
  if (cat == TestMicro && gb->total_frames >= BROM_FRAMES + 10) {
    uint8_t result = bus_read(gb->bus, 0xFF82);
    if (result == 0x01) {
      printf("Passed\n");
    } else if (result == 0xFF) {
      printf("Failed\n");
    } else {
      printf("TEST DID NOT COMPLETE\n");
    }
    ts->done = true;
  }
  if (cat == TestRtc3Basic || cat == TestRtc3Range || cat == TestRtc3Sub) {
    bool btn_down = false;
    bool btn_a = false;
    if (cat == TestRtc3Basic) {
      btn_a = ((f >= 12 && f < 22) != 0);
    } else if (cat == TestRtc3Range) {
      btn_down = ((f >= 12 && f < 22) != 0);
      btn_a = ((f >= 32 && f < 42) != 0);
    } else {
      btn_down = (((f >= 12 && f < 22) || (f >= 32 && f < 42)) != 0);
      btn_a = ((f >= 52 && f < 62) != 0);
    }
    joypad_force_button(gb->joypad, BTN_DOWN, btn_down);
    joypad_force_button(gb->joypad, BTN_A, btn_a);
    if ((cat == TestRtc3Basic &&
         gb->total_frames >= BROM_FRAMES + 22 + (13 * 60)) ||
        (cat == TestRtc3Range &&
         gb->total_frames >= BROM_FRAMES + 42 + (8 * 60)) ||
        (cat == TestRtc3Sub &&
         gb->total_frames >= BROM_FRAMES + 62 + (26 * 60))) {
      ts->done = true;
    }
  }
  if (cat == TestBlarggCpu && gb->total_frames >= BROM_FRAMES + (60 * 60)) {
    ts->done = true;
  }
  if (cat == TestBlarggAudio && gb->total_frames >= BROM_FRAMES + (60 * 60)) {
    ts->done = true;
  }
  if ((cat == TestBlarggCpuTime || cat == TestBlarggMemTime ||
       cat == TestBlarggHaltBug || cat == TestBlarggInterruptTime) &&
      gb->total_frames >= BROM_FRAMES + 120) {
    ts->done = true;
  }
  if (cat == TestBlarggMemTime2 && gb->total_frames >= BROM_FRAMES + 240) {
    ts->done = true;
  }
  if (cat == TestBlarggOamBug && gb->total_frames >= BROM_FRAMES + 1260) {
    ts->done = true;
  }
  if (cat == TestBlarggCgbSound && gb->total_frames >= BROM_FRAMES + 2220) {
    ts->done = true;
  }
  if (cat == TestBully && gb->total_frames >= BROM_FRAMES + 30) {
    ts->done = true;
  }
  if (cat == TestGambatte && gb->total_frames >= BROM_FRAMES + 15) {
    ts->done = true;
  }
  if (cat == TestLittle && gb->total_frames >= BROM_FRAMES + 30) {
    ts->done = true;
  }
  if (cat == TestMbc3 && gb->total_frames >= BROM_FRAMES + 60) {
    ts->done = true;
  }
  if (cat == TestScribble && gb->total_frames >= BROM_FRAMES + 10) {
    ts->done = true;
  }
  if (cat == TestStrike && gb->total_frames >= BROM_FRAMES + 30) {
    ts->done = true;
  }
  if (cat == TestTurtle && gb->total_frames >= BROM_FRAMES + 30) {
    ts->done = true;
  }
}

int main(int argc, char* argv[]) {
  char* rom_name = NULL;
  char* screenshot_path = NULL;
  int test_category = -1;
  bool disassemble_enable = false;
  uint64_t stop_frame = (uint64_t)-1;
  uint16_t watch_addrs[8];
  uint8_t watch_count = 0;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--version")) {
      printf("gbemu " GBEMU_VERSION "\n");
      return 0;
    }
    if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      printf("Usage: gbemu_headless -r <rom.gb> [options]\n\n");
      printf("Options:\n");
      printf("  -r, --rom <file>        ROM file to run\n");
      printf("  -t, --test <category>   Test category for automated testing\n");
      printf("  -s, --screenshot <file> Save screenshot to PNG file\n");
      printf("  -f, --stop-frame <N>    Stop after N total frames\n");
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
    }
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc) {
      {
        rom_name = argv[++i];
      }
    } else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) &&
               i + 1 < argc) {
      char* s = argv[++i];
      if (!strcmp(s, "age")) {
        {
          test_category = TestAge;
        }
      } else if (!strcmp(s, "blargg_cpu")) {
        {
          test_category = TestBlarggCpu;
        }
      } else if (!strcmp(s, "blargg_audio")) {
        {
          test_category = TestBlarggAudio;
        }
      } else if (!strcmp(s, "blargg_cpu_time")) {
        {
          test_category = TestBlarggCpuTime;
        }
      } else if (!strcmp(s, "blargg_mem_time")) {
        {
          test_category = TestBlarggMemTime;
        }
      } else if (!strcmp(s, "blargg_halt_bug")) {
        {
          test_category = TestBlarggHaltBug;
        }
      } else if (!strcmp(s, "blargg_interrupt_time")) {
        {
          test_category = TestBlarggInterruptTime;
        }
      } else if (!strcmp(s, "blargg_mem_time2")) {
        {
          test_category = TestBlarggMemTime2;
        }
      } else if (!strcmp(s, "blargg_oam_bug")) {
        {
          test_category = TestBlarggOamBug;
        }
      } else if (!strcmp(s, "blargg_cgb_sound")) {
        {
          test_category = TestBlarggCgbSound;
        }
      } else if (!strcmp(s, "bully")) {
        {
          test_category = TestBully;
        }
      } else if (!strcmp(s, "acid2")) {
        {
          test_category = TestAcid2;
        }
      } else if (!strcmp(s, "gambatte")) {
        {
          test_category = TestGambatte;
        }
      } else if (!strcmp(s, "micro")) {
        {
          test_category = TestMicro;
        }
      } else if (!strcmp(s, "little")) {
        {
          test_category = TestLittle;
        }
      } else if (!strcmp(s, "mbc3")) {
        {
          test_category = TestMbc3;
        }
      } else if (!strcmp(s, "mealybug")) {
        {
          test_category = TestMealybug;
        }
      } else if (!strcmp(s, "mooneye")) {
        {
          test_category = TestMooneye;
        }
      } else if (!strcmp(s, "same")) {
        {
          test_category = TestSame;
        }
      } else if (!strcmp(s, "rtc3_basic")) {
        {
          test_category = TestRtc3Basic;
        }
      } else if (!strcmp(s, "rtc3_range")) {
        {
          test_category = TestRtc3Range;
        }
      } else if (!strcmp(s, "rtc3_sub")) {
        {
          test_category = TestRtc3Sub;
        }
      } else if (!strcmp(s, "scribble")) {
        {
          test_category = TestScribble;
        }
      } else if (!strcmp(s, "strike")) {
        {
          test_category = TestStrike;
        }
      } else if (!strcmp(s, "turtle")) {
        {
          test_category = TestTurtle;
        }
      } else {
        (void)fprintf(stderr, "UNKNOWN TEST CATEGORY: %s\n", s);
        return 1;
      }
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly")) {
      {
        disassemble_enable = true;
      }
    } else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--screenshot")) &&
               i + 1 < argc) {
      {
        screenshot_path = argv[++i];
      }
    } else if ((!strcmp(argv[i], "-f") || !strcmp(argv[i], "--stop-frame")) &&
               i + 1 < argc) {
      {
        stop_frame = (uint64_t)strtoull(argv[++i], NULL, 10);
      }
    } else if ((!strcmp(argv[i], "-w") || !strcmp(argv[i], "--watch")) &&
               i + 1 < argc) {
      if (watch_count < 8) {
        watch_addrs[watch_count++] = (uint16_t)strtol(argv[++i], NULL, 16);
      } else {
        i++;
      }
    } else {
      (void)fprintf(stderr, "UNKNOWN COMMAND LINE OPTION: %s\n", argv[i]);
      return 1;
    }
  }
  if (!rom_name) {
    (void)fprintf(stderr, "MUST PROVIDE ROM FILE\n");
    return 1;
  }
  struct GBemu* gb =
      gbemu_init(rom_name, disassemble_enable, watch_addrs, watch_count);
  if (!gb) {
    return 1;
  }
  struct TestState ts = test_init(test_category);
  while (!ts.done) {
    if (gbemu_step_frame(gb) == -1) {
      gbemu_free(gb);
      return 1;
    }
    handle_test_frame(gb, &ts);
    if (gb->total_frames >= stop_frame) {
      ts.done = true;
    }
  }
  if (screenshot_path && write_screenshot(gb->ppu, screenshot_path)) {
    return 1;
  }
  gbemu_free(gb);
  return 0;
}
