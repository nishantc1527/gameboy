#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gbemu/gbemu.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static const uint8_t ACID_COLORS[5] = {0xFF, 0xAA, 0x55, 0x00, 0x00};

static int write_screenshot(struct Ppu* ppu, const char* path) {
  uint8_t buf[144][160];
  for (int y = 0; y < 144; y++)
    for (int x = 0; x < 160; x++) buf[y][x] = ACID_COLORS[ppu->dsp[y][x]];
  if (!stbi_write_png(path, 160, 144, 1, buf, 160)) {
    fprintf(stderr, "Failed to write screenshot: %s\n", path);
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  char* rom_name = NULL;
  char* screenshot_path = NULL;
  int test_category = -1;
  int disassemble_enable = 0;
  uint16_t watch_addrs[8];
  uint8_t watch_count = 0;
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc)
      rom_name = argv[++i];
    else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) &&
             i + 1 < argc) {
      char* s = argv[++i];
      if (!strcmp(s, "age")) test_category = TEST_AGE;
      else if (!strcmp(s, "blargg_cpu")) test_category = TEST_BLARGG_CPU;
      else if (!strcmp(s, "blargg_audio")) test_category = TEST_BLARGG_AUDIO;
      else if (!strcmp(s, "blargg_cpu_time"))
        test_category = TEST_BLARGG_CPU_TIME;
      else if (!strcmp(s, "blargg_mem_time"))
        test_category = TEST_BLARGG_MEM_TIME;
      else if (!strcmp(s, "bully")) test_category = TEST_BULLY;
      else if (!strcmp(s, "acid2")) test_category = TEST_ACID2;
      else if (!strcmp(s, "gambatte")) test_category = TEST_GAMBATTE;
      else if (!strcmp(s, "micro")) test_category = TEST_MICRO;
      else if (!strcmp(s, "little")) test_category = TEST_LITTLE;
      else if (!strcmp(s, "mbc3")) test_category = TEST_MBC3;
      else if (!strcmp(s, "mealybug")) test_category = TEST_MEALYBUG;
      else if (!strcmp(s, "mooneye")) test_category = TEST_MOONEYE;
      else if (!strcmp(s, "same")) test_category = TEST_SAME;
      else if (!strcmp(s, "scribble")) test_category = TEST_SCRIBBLE;
      else if (!strcmp(s, "strike")) test_category = TEST_STRIKE;
      else if (!strcmp(s, "turtle")) test_category = TEST_TURTLE;
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
      else i++;
    } else {
      fprintf(stderr, "UNKNOWN COMMAND LINE OPTION: %s\n", argv[i]);
      return 1;
    }
  }
  if (!rom_name) {
    fprintf(stderr, "MUST PROVIDE ROM FILE\n");
    return 1;
  }
  gbemu* gb = gbemu_init(rom_name, test_category, (uint8_t)disassemble_enable,
                         watch_addrs, watch_count);
  if (!gb) return 1;
  while (!gb->bdone) {
    if (gbemu_step_frame(gb) == -1) {
      gbemu_free(gb);
      return 1;
    }
  }
  if (screenshot_path && write_screenshot(gb->ppu, screenshot_path)) return 1;
  gbemu_free(gb);
  return 0;
}
