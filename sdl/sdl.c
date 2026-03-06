#define SDL_MAIN_USE_CALLBACKS
#include "gbemu/sdl.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>

#include "gbemu/gbemu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/settings.h"

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
  settings_load(&g_settings);
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "INITIALIZING\n");
  gbemu* gb = gbemu_init(rom_name, g_settings.boot_rom, -1, disassemble_enable,
                         NULL, 0);
  if (init_window(mmu_get_rom_title(gb->mmu))) {
    gbemu_free(gb);
    return SDL_APP_FAILURE;
  }
  snprintf(g_settings.last_rom, sizeof(g_settings.last_rom), "%s", rom_name);
  settings_add_recent_rom(&g_settings, rom_name);
  settings_save(&g_settings);
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
  if (tot_ticks >= frame_ticks * 2) tot_ticks = frame_ticks;
  if (tot_ticks >= frame_ticks) {
    if (gbemu_step_frame(gb) == -1) return SDL_APP_FAILURE;
    push_audio(gb->apu);
    tot_ticks -= frame_ticks;
    update_input(gb->ppu, gb->mmu);
    render(gb->ppu);
    // draw_ui();
    SDL_RenderPresent(rnd);
  } else {
    Uint64 remaining = frame_ticks - tot_ticks;
    Uint64 sleep_ms = (remaining * 1000) / perf_freq;
    if (sleep_ms > 1) SDL_Delay((Uint32)(sleep_ms - 1));
  }
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
  settings_save(&g_settings);
  gbemu_free(appstate);
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DONE\n");
}
