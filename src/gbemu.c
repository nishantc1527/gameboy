#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"
#include "rom_locs.h"
#include "rust.h"

char* rom_name = NULL;
uint8_t test_category = -1;
uint8_t headless = 0;
uint8_t disassemble = 0;
char* test_out;
int test_out_idx;

SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom rom file] [-t/--test test rom category] "
               "[-h/--headless] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate __attribute__((unused)),
                          int argc __attribute__((unused)),
                          char* argv[] __attribute__((unused))) {
  if (argc < 2) return usage();
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc)
      rom_name = argv[++i];
    else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) &&
             i + 1 < argc) {
      char* s = argv[++i];
      if (!strcmp(s, "blargg"))
        test_category = TEST_BLARGG;
      else if (!strcmp(s, "mooneye"))
        test_category = TEST_MOONEYE;
      else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNKNOWN TEST: %s\n", s);
        return SDL_APP_FAILURE;
      }
      test_out = (char*)malloc(1000 * sizeof(char));
      test_out[0] = '\0';
      test_out_idx = 0;
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--headless"))
      headless = 1;
    else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble = 1;
    else {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNKNOWN COMMAND LINE OPTION %s\n",
                   argv[i]);
      return usage();
    }
  }
  if (!rom_name) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "MUST PROVIDE ROM FILE\n");
    return SDL_APP_FAILURE;
  }

  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "INITIALIZING\n");
  init_reg();
  FILE* boot_rom_file = fopen(BOOT_ROM_FILE, "rb");
  if (!boot_rom_file) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT OPEN BOOT ROM\n");
    return SDL_APP_FAILURE;
  }
  FILE* rom_file = fopen(rom_name, "rb");
  if (!fread(brom, 0x100, 1, boot_rom_file)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT READ BOOT ROM FILE\n");
    return SDL_APP_FAILURE;
  }
  fclose(boot_rom_file);
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOADED BOOT ROM\n");
  if (!rom_file) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT OPEN ROM\n");
    return SDL_APP_FAILURE;
  }
  if (!fread(mem, 0x8000, 1, rom_file)) {  // Put only first bank in memory
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT READ ROM FILE\n");
    return SDL_APP_FAILURE;
  }
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOADED ROM\n");
  if (get_rom_info()) {
    fclose(rom_file);
    return SDL_APP_FAILURE;
  }
  fclose(rom_file);
  rom_file = fopen(rom_name, "rb");
  if (rom_file) {
    if (!fread(rom, (1LL << rom_size) * 0x8000, 1, rom_file)) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT READ ROM FILE\n");
      return SDL_APP_FAILURE;
    }
    fclose(rom_file);
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT OPEN ROM\n");
    return SDL_APP_FAILURE;
  }
  if (init_window()) return SDL_APP_FAILURE;
  init_ppu();
  load();
  if (pokemon_enabled) p_init_data();

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate __attribute__((unused))) {
  Uint64 curr = SDL_GetPerformanceCounter();
  Uint64 elapsed = curr - prev_time;
  prev_time = curr;
  tot_ticks += elapsed;
  scn = 0;
  Uint64 frame_cyc = (Uint64)SCANLINE_LEN * (Uint64)SCANLINES;
  Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (tot_ticks >= frame_ticks * 10) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Performance drop by %d frames\n",
    // (int)(tot_ticks / frame_ticks));
    tot_ticks = 0;
  } else if (tot_ticks >= frame_ticks) {
    frame = 0;
    while (!frame) {
      const int cyc = exec_instr();
      if (cyc == -1) return SDL_APP_FAILURE;
      PC++;
      scn += cyc;
      if (scn >= SCANLINE_LEN) {
        do_scanline();
        scn -= SCANLINE_LEN;
      }
      update_lcd();
      update_timer(cyc);
      check_dma();
      check_interrupt();
      update_input();
    }
    tot_ticks -= frame_ticks;
  }
  if (test_category == TEST_BLARGG) {
    char* results[2] = {"Passed", "Failed"};
    for (int i = 0; i < 2; i++) {
      if (strstr(test_out, results[i])) {
        printf("%s", results[i]);
        return SDL_APP_SUCCESS;
      }
    }
  }
  if (!headless) {
    render();
    // draw_ui();
    SDL_RenderPresent(rnd);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate __attribute__((unused)),
                           SDL_Event* event) {
  if (handle_input(event)) return SDL_APP_SUCCESS;
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate __attribute__((unused)), SDL_AppResult result) {
  switch (result) {
    case SDL_APP_SUCCESS:
    case SDL_APP_CONTINUE:
      // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SUCCESS\n");
      break;
    case SDL_APP_FAILURE:
      // SDL_LogError(SDL_LOG_CATEGORY_ERROR, "FAILURE\n");
      break;
  }
  save();
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DONE\n");
}
