#include <stdio.h>
#include <string.h>

#include "gbemu/gbemu.h"

static void usage(void) {
  fprintf(stderr,
          "Usage: gbemu_headless [-r/--rom <rom file>] [-t/--test "
          "<blargg|mooneye>] "
          "[-d/disassembly]\n");
}

int main(int argc, char* argv[]) {
  char* rom_name = NULL;
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
      else {
        fprintf(stderr, "UNKNOWN TEST CATEGORY: %s\n", s);
        usage();
        return 1;
      }
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble_enable = 1;
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
  gbemu* gb = gbemu_init(rom_name, test_category, (uint8_t)disassemble_enable);
  if (!gb) return 1;

  while (!gb->b_done) {
    if (gbemu_step_frame(gb) == -1) {
      gbemu_free(gb);
      return 1;
    }
  }

  gbemu_free(gb);
  return 0;
}
