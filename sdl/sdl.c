#define SDL_MAIN_USE_CALLBACKS
#include "gbemu/sdl.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>

#include "gbemu/gbemu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"

static SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom <rom file>] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc,
                          char* argv[] __attribute__((unused))) {
  if (argc < 1) return usage();
  char* rom_name = NULL;
  uint8_t disassemble_enable = 0;
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc)
      rom_name = argv[++i];
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
  gbemu* gb = gbemu_init(rom_name, -1, disassemble_enable, NULL, 0);
  if (init_window(mmu_get_rom_title(gb->mmu))) {
    gbemu_free(gb);
    return SDL_APP_FAILURE;
  }
  if (pokemon_enabled) p_init_data(gb->mmu);
  *appstate = gb;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  gbemu* gb = appstate;
  if (gb->bdone) return SDL_APP_SUCCESS;
  Uint64 curr = SDL_GetPerformanceCounter();
  Uint64 elapsed = curr - prev_time;
  prev_time = curr;
  tot_ticks += elapsed;
  gb->ppu->scn = 0;
  Uint64 frame_cyc = (Uint64)SCANLINE_LEN * (Uint64)SCANLINES;
  Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (tot_ticks >= frame_ticks * 10) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Performance drop by %d frames\n",
    // (int)(tot_ticks / frame_ticks));
    tot_ticks = 0;
  } else if (tot_ticks >= frame_ticks) {
    if (gbemu_step_frame(gb) == -1) return SDL_APP_FAILURE;
    tot_ticks -= frame_ticks;
  }
  update_input(gb->ppu, gb->mmu);
  render(gb->ppu);
  // draw_ui();
  SDL_RenderPresent(rnd);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  gbemu* gb = appstate;
  if (handle_input(gb->ppu, event)) return SDL_APP_SUCCESS;
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
