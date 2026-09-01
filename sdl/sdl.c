#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbemu/apu.h"
#include "gbemu/core.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "rust.h"
#include "sdl_private.h"

int pokemon_enabled;

static SDL_AppResult usage() {
  SDL_LogError(SDL_LOG_CATEGORY_ERROR,
               "Usage: gbemu [-r/--rom <rom file>] [-d/--disassembly]\n");
  return SDL_APP_FAILURE;
}

void pokemon_resize_window(void) {
  int w = POKEMON_LEFT_W + SCRN_WIDTH * g_settings.scale + POKEMON_RIGHT_W;
  int h = menu_bar_height + POKEMON_HEADER_H + SCRN_HEIGHT * g_settings.scale +
          POKEMON_BOTTOM_H;
  SDL_SetWindowSize(win, w, h);
  SDL_GetWindowSize(win, &win_width, &win_height);
}

void pokemon_restore_window(void) {
  SDL_SetWindowSize(win, SCRN_WIDTH * g_settings.scale,
                    SCRN_HEIGHT * g_settings.scale + menu_bar_height);
  SDL_GetWindowSize(win, &win_width, &win_height);
}

static int load_rom(struct AppState* state, const char* path, uint8_t dis) {
  FILE* f = fopen(path, "r");
  if (!f) {
    SDL_snprintf(state->rom_error, sizeof(state->rom_error),
                 "ROM not found: %s", path);
    settings_remove_recent_rom(&g_settings, path);
    settings_save(&g_settings);
    return 1;
  }
  (void)fclose(f);
  state->rom_error[0] = '\0';
  bool was_pokemon = pokemon_enabled != 0;
  gbemu_free(state->gb);
  pokemon_enabled = 0;
  state->gb = gbemu_init((char*)path, dis != 0U, NULL, 0);
  if (!state->gb) {
    SDL_snprintf(state->rom_error, sizeof(state->rom_error),
                 "Failed to load ROM: %s", path);
    if (was_pokemon) pokemon_restore_window();
    return 1;
  }
  const char* title = mmu_get_rom_title(state->gb->mmu);
  set_window_title_rom(title);
  settings_add_recent_rom(&g_settings, path);
  settings_save(&g_settings);
  if (pokemon_check(title)) {
    pokemon_enabled = 1;
    pokemon_init(state->gb->mmu);
    state->pokemon_focused_slot = 0;
    state->pokemon_session_start = SDL_GetTicks();
    if (!was_pokemon) pokemon_resize_window();
  } else {
    if (was_pokemon) pokemon_restore_window();
  }
  return 0;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  if (argc < 1) {
    return usage();
  }
  char* rom_name = NULL;
  bool disassemble_enable = false;
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rom")) && i + 1 < argc) {
      {
        rom_name = argv[++i];
      }
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disassembly")) {
      {
        disassemble_enable = true;
      }
    } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      SDL_Log(
          "Usage: gbemu [-r <rom.gb>] [-d]\n\n"
          "Options:\n"
          "  -r, --rom <file>    ROM file to load on startup\n"
          "  -d, --disassembly   Print per-instruction disassembly\n"
          "  -h, --help          Show this help\n");
      return SDL_APP_SUCCESS;
    } else {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unknown option: %s\n", argv[i]);
      return usage();
    }
  }

  settings_load(&g_settings);

  struct AppState* state = calloc(1, sizeof(struct AppState));
  if (!state) {
    return SDL_APP_FAILURE;
  }
  state->rebinding_control = -1;
  *appstate = state;

  if (init_window("gbemu")) {
    free(state);
    return SDL_APP_FAILURE;
  }

  if (rom_name) {
    if (load_rom(state, rom_name, (uint8_t)disassemble_enable)) {
      return SDL_APP_FAILURE;
    }
    (void)snprintf(g_settings.last_rom, sizeof(g_settings.last_rom), "%s",
                   rom_name);
    settings_save(&g_settings);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  struct AppState* state = appstate;

  if (state->close_requested) {
    state->close_requested = false;
    gbemu_free(state->gb);
    state->gb = NULL;
    if (pokemon_enabled) {
      pokemon_enabled = 0;
      pokemon_restore_window();
    }
    SDL_SetWindowTitle(win, "gbemu");
  }

  if (state->pending_rom[0]) {
    char path[512];
    SDL_snprintf(path, sizeof(path), "%s", state->pending_rom);
    state->pending_rom[0] = '\0';
    load_rom(state, path, 0);
  }

  static Uint64 perf_freq = 0;
  static Uint64 next_frame_time = 0;
  if (!perf_freq) {
    perf_freq = SDL_GetPerformanceFrequency();
    next_frame_time = SDL_GetPerformanceCounter();
  }
  const Uint64 frame_cyc =
      (Uint64)PPU_CYCLES_PER_LINE * (Uint64)PPU_TOTAL_LINES;
  const Uint64 frame_ticks = (frame_cyc * perf_freq) / CPU_FREQ;
  if (state->gb && !state->gb->is_paused && state->gb->fast_forward) {
    for (int i = 0; i < 4; i++) {
      if (gbemu_step_frame(state->gb) == -1) {
        return SDL_APP_FAILURE;
      }
    }
    apu_discard_samples(state->gb->apu);
    next_frame_time = SDL_GetPerformanceCounter();
  } else {
    Uint64 now = SDL_GetPerformanceCounter();
    if (now < next_frame_time) {
      SDL_Delay(1);
      return SDL_APP_CONTINUE;
    }
    next_frame_time += frame_ticks;
    if (next_frame_time < now) {
      next_frame_time = now;
    }

    if (state->gb && !state->gb->is_paused) {
      if (gbemu_step_frame(state->gb) == -1) {
        return SDL_APP_FAILURE;
      }
      push_audio(state->gb->apu);
    }
  }

  SDL_SetRenderDrawColor(rnd, 20, 20, 20, 255);
  SDL_RenderClear(rnd);
  if (state->gb) {
    render(state->gb->ppu);
  }
  draw_ui(state);
  SDL_RenderPresent(rnd);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  struct AppState* state = appstate;
  if (handle_input(state, event)) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  (void)result;
  struct AppState* state = appstate;
  if (!state) {
    return;
  }
  settings_save(&g_settings);
  gbemu_free(state->gb);
  imgui_shutdown();
  free(state);
}
