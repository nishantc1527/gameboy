#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"
#include "rom_locs.h"
#include "rust.h"

Mmu* mmu = NULL;

char* rom_name = NULL;
int test_category = -1;
uint8_t headless = 0;
uint8_t disassemble = 0;
uint8_t done = 0;

SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom rom file] [-t/--test test rom category] "
               "[-h/--headless] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate __attribute__((unused)),
                          int argc __attribute__((unused)),
                          char* argv[] __attribute__((unused))) {
  if (argc < 1) return usage();
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
  mmu = mmu_init(rom_name, BOOT_ROM_FILE, (int8_t)test_category);
  if (init_window()) return SDL_APP_FAILURE;
  init_ppu();
  init_apu();
  mmu_load(mmu);
  if (pokemon_enabled) p_init_data();
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate __attribute__((unused))) {
  if (done) return SDL_APP_SUCCESS;
  Uint64 curr = SDL_GetPerformanceCounter();
  Uint64 elapsed = curr - prev_time;
  prev_time = curr;
  tot_ticks += elapsed;
  scn = 0;
  Uint64 frame_cyc = (Uint64)SCANLINE_LEN * (Uint64)SCANLINES;
  Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (!headless && tot_ticks >= frame_ticks * 10) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Performance drop by %d frames\n",
    // (int)(tot_ticks / frame_ticks));
    tot_ticks = 0;
  } else if (headless || tot_ticks >= frame_ticks) {
    frame = 0;
    while (!frame) {
      const int cyc = exec_instr();
      if (cyc == -1) return SDL_APP_FAILURE;
      PC++;
      scn = (uint16_t)(scn + cyc);
      if (scn >= SCANLINE_LEN) {
        do_scanline();
        scn -= SCANLINE_LEN;
      }
      update_lcd();
      update_timer((uint8_t)cyc);
      check_dma();
      check_interrupt();
      upd_apu();
    }
    tot_ticks -= frame_ticks;
  }
  update_input();
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
  if (mmu) {
    mmu_save(mmu);
    mmu_free(mmu);
  }
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DONE\n");
}
