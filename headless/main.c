#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gbemu/gbemu.h"
#include "stb_image_write.h"

static void usage(void) {
  fprintf(stderr,
          "Usage: gbemu_headless [-r/--rom <rom file>] [-t/--test "
          "<blargg|mooneye|blargg_audio|acid2>] "
          "[-d/--disassembly] [-s/--screenshot <out.png>]\n");
}

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
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc)
      rom_name = argv[++i];
    else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) &&
             i + 1 < argc) {
      char* s = argv[++i];
      if (!strcmp(s, "blargg")) test_category = TEST_BLARGG;
      else if (!strcmp(s, "mooneye"))
        test_category = TEST_MOONEYE;
      else if (!strcmp(s, "blargg_audio"))
        test_category = TEST_BLARGG_AUDIO;
      else if (!strcmp(s, "acid2"))
        test_category = TEST_ACID2;
      else {
        fprintf(stderr, "UNKNOWN TEST CATEGORY: %s\n", s);
        usage();
        return 1;
      }
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble_enable = 1;
    else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--screenshot")) &&
             i + 1 < argc)
      screenshot_path = argv[++i];
    else {
      fprintf(stderr, "UNKNOWN COMMAND LINE OPTION: %s\n", argv[i]);
      usage();
      return 1;
    }
  }
  if (!rom_name) {
    fprintf(stderr, "MUST PROVIDE ROM FILE\n");
    usage();
    return 1;
  }
  if ((!screenshot_path && test_category == TEST_ACID2) ||
      (screenshot_path && test_category != TEST_ACID2)) {
    printf("MUST PROVIDE TEST FOR SCREENSHOT\n");
    return 1;
  }
  gbemu* gb = gbemu_init(rom_name, test_category, (uint8_t)disassemble_enable);
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
