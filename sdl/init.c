#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>

#include "gbemu/ppu.h"
#include "gbemu/sdl.h"

// TODO sdl state
int win_width, win_height;
SDL_Texture* txt = NULL;

SDL_Window* win;
SDL_Renderer* rnd;
struct nk_context* ctx;

Uint64 perf_freq;
Uint64 prev_time;
Uint64 tot_ticks = 0;

SDL_Keycode g_controls[CTRL_COUNT];

void populate_controls(void) {
  g_controls[CTRL_A] = SDL_GetKeyFromName(g_settings.key_a);
  g_controls[CTRL_B] = SDL_GetKeyFromName(g_settings.key_b);
  g_controls[CTRL_START] = SDL_GetKeyFromName(g_settings.key_start);
  g_controls[CTRL_SELECT] = SDL_GetKeyFromName(g_settings.key_select);
  g_controls[CTRL_UP] = SDL_GetKeyFromName(g_settings.key_up);
  g_controls[CTRL_DOWN] = SDL_GetKeyFromName(g_settings.key_down);
  g_controls[CTRL_LEFT] = SDL_GetKeyFromName(g_settings.key_left);
  g_controls[CTRL_RIGHT] = SDL_GetKeyFromName(g_settings.key_right);
  g_controls[CTRL_PAUSE] = SDL_GetKeyFromName(g_settings.key_pause);
  g_controls[CTRL_SCREENSHOT] = SDL_GetKeyFromName(g_settings.key_screenshot);
}

int init_window(const char* rom_title) {
  populate_controls();
  if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO |
                SDL_INIT_GAMEPAD)) {
    SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "ERROR INITIALIZING SDL: %s\n",
                 SDL_GetError());
    return 1;
  }
  if (!SDL_CreateWindowAndRenderer(rom_title, SCRN_WIDTH * g_settings.scale,
                                   SCRN_HEIGHT * g_settings.scale, 0, &win,
                                   &rnd)) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                 "ERROR CREATING WINDOW & RENDERER: %s\n", SDL_GetError());
    return 1;
  }
  SDL_SetRenderVSync(rnd, 0);
  SDL_SetRenderLogicalPresentation(rnd, SCRN_WIDTH, SCRN_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_GetWindowSize(win, &win_width, &win_height);
  txt = SDL_CreateTexture(rnd, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
  SDL_SetTextureScaleMode(txt, SDL_SCALEMODE_NEAREST);
  if (!txt) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNABLE TO CREATE TEXTURE\n");
    return 0;
  }
  ctx = nk_sdl_init(win, rnd, nk_sdl_allocator());
  nk_sdl_style_set_debug_font(ctx);
  perf_freq = SDL_GetPerformanceFrequency();
  prev_time = SDL_GetPerformanceCounter();
  init_audio();
  return 0;
}
