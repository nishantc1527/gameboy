#define SDL_MAIN_USE_CALLBACKS
#include "gbemu/gbemu.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <stdlib.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"
#include "rom_locs.h"
#include "rust.h"

GbEmu* gbemu_init(char* rom_name, int test_category, uint8_t headless,
                  uint8_t disassemble_enable) {
  GbEmu* gb = malloc(sizeof(GbEmu));
  gb->rom_name = rom_name;
  gb->test_category = test_category;
  gb->headless = headless;
  gb->disassemble_enable = disassemble_enable;
  gb->b_done = 0;
  gb->cpu = init_cpu();
  gb->mmu = mmu_init(rom_name, BOOT_ROM_FILE, (int8_t)test_category);
  gb->ppu = init_ppu();
  mmu_load(gb->mmu);
  return gb;
}

void gbemu_free(GbEmu* gb) {
  if (!gb) return;
  free(gb->cpu);
  free(gb->ppu);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}

SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom rom file] [-t/--test test rom category] "
               "[-h/--headless] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc,
                          char* argv[] __attribute__((unused))) {
  char* rom_name = NULL;
  int test_category = -1;
  uint8_t headless = 0;
  uint8_t disassemble_enable = 0;

  if (argc < 1) return usage();
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNKNOWN TEST: %s\n", s);
        return SDL_APP_FAILURE;
      }
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--headless"))
      headless = 1;
    else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble_enable = 1;
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
  GbEmu* gb = gbemu_init(rom_name, test_category, headless, disassemble_enable);
  if (init_window(mmu_get_rom_title(gb->mmu), gb->headless)) {
    gbemu_free(gb);
    return SDL_APP_FAILURE;
  }
  if (pokemon_enabled) p_init_data(gb->mmu);
  *appstate = gb;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  GbEmu* gb = appstate;
  if (gb->b_done) return SDL_APP_SUCCESS;
  Uint64 curr = SDL_GetPerformanceCounter();
  Uint64 elapsed = curr - prev_time;
  prev_time = curr;
  tot_ticks += elapsed;
  gb->ppu->scn = 0;
  Uint64 frame_cyc = (Uint64)SCANLINE_LEN * (Uint64)SCANLINES;
  Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (!gb->headless && tot_ticks >= frame_ticks * 10) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Performance drop by %d frames\n",
    // (int)(tot_ticks / frame_ticks));
    tot_ticks = 0;
  } else if (gb->headless || tot_ticks >= frame_ticks) {
    gb->ppu->frame = 0;
    while (!gb->ppu->frame) {
      const int cyc = step(gb->cpu, gb->mmu, gb->disassemble_enable,
                           gb->test_category, &gb->b_done);
      if (cyc == -1) return SDL_APP_FAILURE;
      gb->ppu->scn = (uint16_t)(gb->ppu->scn + cyc);
      if (gb->ppu->scn >= SCANLINE_LEN) {
        do_scanline(gb->ppu, gb->mmu);
        gb->ppu->scn -= SCANLINE_LEN;
      }
      update_lcd(gb->ppu, gb->mmu);
      update_timer(gb->cpu, gb->mmu, (uint8_t)cyc);
      check_dma(gb->mmu);
      check_interrupt(gb->cpu, gb->mmu);
    }
    tot_ticks -= frame_ticks;
  }
  update_input(gb->ppu, gb->mmu);
  if (!gb->headless) {
    render(gb->ppu);
    // draw_ui();
    SDL_RenderPresent(rnd);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  GbEmu* gb = appstate;
  if (handle_input(gb->ppu, event, gb->headless)) return SDL_APP_SUCCESS;
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  switch (result) {
    case SDL_APP_SUCCESS:
    case SDL_APP_CONTINUE:
      // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SUCCESS\n");
      break;
    case SDL_APP_FAILURE:
      // SDL_LogError(SDL_LOG_CATEGORY_ERROR, "FAILURE\n");
      break;
  }
  gbemu_free(appstate);
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DONE\n");
}
