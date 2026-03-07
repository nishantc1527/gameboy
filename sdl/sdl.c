#define SDL_MAIN_USE_CALLBACKS
#include "gbemu/sdl.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbemu/gbemu.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/settings.h"

static SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom <rom file>] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

static int load_rom(AppState* state, const char* path, uint8_t dis) {
  gbemu_free(state->gb);
  state->gb = gbemu_init((char*)path, g_settings.boot_rom, -1, dis, NULL, 0);
  if (!state->gb) return 1;
  set_window_title_rom(mmu_get_rom_title(state->gb->mmu));
  settings_add_recent_rom(&g_settings, path);
  settings_save(&g_settings);
  if (pokemon_enabled) p_init_data(state->gb->mmu);
  return 0;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  if (argc < 1) return usage();
  char* rom_name = NULL;
  uint8_t disassemble_enable = 0;
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc)
      rom_name = argv[++i];
    else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly"))
      disassemble_enable = 1;
    else {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unknown option: %s\n", argv[i]);
      return usage();
    }
  }

  settings_load(&g_settings);

  AppState* state = calloc(1, sizeof(AppState));
  if (!state) return SDL_APP_FAILURE;
  *appstate = state;

  if (init_window("gbemu")) {
    free(state);
    return SDL_APP_FAILURE;
  }

  if (rom_name) {
    if (load_rom(state, rom_name, disassemble_enable)) return SDL_APP_FAILURE;
    snprintf(g_settings.last_rom, sizeof(g_settings.last_rom), "%s", rom_name);
    settings_save(&g_settings);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  AppState* state = appstate;

  if (state->pending_rom[0]) {
    load_rom(state, state->pending_rom, 0);
    state->pending_rom[0] = '\0';
  }

  if (state->gb && state->gb->bdone) return SDL_APP_SUCCESS;

  Uint64 curr = SDL_GetPerformanceCounter();
  Uint64 elapsed = curr - prev_time;
  prev_time = curr;
  tot_ticks += elapsed;

  Uint64 frame_cyc = (Uint64)SCANLINE_LEN * (Uint64)SCANLINES;
  Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (tot_ticks >= frame_ticks * 2) tot_ticks = frame_ticks;

  if (tot_ticks >= frame_ticks) {
    tot_ticks -= frame_ticks;

    SDL_SetRenderDrawColor(rnd, 20, 20, 20, 255);
    SDL_RenderClear(rnd);

    if (state->gb) {
      if (!state->gb->paused) {
        int frames = state->gb->fast_forward ? 4 : 1;
        for (int i = 0; i < frames; i++) {
          if (gbemu_step_frame(state->gb) == -1) return SDL_APP_FAILURE;
        }
        push_audio(state->gb->apu);
      }
      update_input(state->gb->ppu, state->gb->mmu);
      render(state->gb->ppu);
    }

    draw_ui(state);
    SDL_RenderPresent(rnd);
  } else {
    Uint64 remaining = frame_ticks - tot_ticks;
    Uint64 sleep_ms = (remaining * 1000) / perf_freq;
    if (sleep_ms > 1) SDL_Delay((Uint32)(sleep_ms - 1));
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  AppState* state = appstate;
  if (handle_input(state, event)) return SDL_APP_SUCCESS;
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  (void)result;
  AppState* state = appstate;
  if (!state) return;
  settings_save(&g_settings);
  gbemu_free(state->gb);
  free(state);
}
