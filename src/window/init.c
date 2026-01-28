#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>

#include "display.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"

int win_width, win_height;

SDL_Window* win;
SDL_Renderer* rnd;
struct nk_context* ctx;

Uint64 perf_freq;
Uint64 prev_time;
Uint64 tot_ticks = 0;

SDL_Texture* txt = NULL;

int init_window(void) {
  if (!headless) {
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "ERROR INITIALIZING SDL: %s\n",
                   SDL_GetError());
      return 1;
    }
    if (!SDL_CreateWindowAndRenderer(rom_title, SCRN_WIDTH * SCALE_X,
                                     SCRN_HEIGHT * SCALE_Y, 0, &win, &rnd)) {
      SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                   "ERROR CREATING WINDOW & RENDERER: %s\n", SDL_GetError());
      return 1;
    }
    SDL_GetWindowSize(win, &win_width, &win_height);
    txt =
        SDL_CreateTexture(rnd, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
    SDL_SetTextureScaleMode(txt, SDL_SCALEMODE_NEAREST);
    if (!txt) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNABLE TO CREATE TEXTURE\n");
      return 0;
    }
    ctx = nk_sdl_init(win, rnd, nk_sdl_allocator());
    nk_sdl_style_set_debug_font(ctx);
  }
  perf_freq = SDL_GetPerformanceFrequency();
  prev_time = SDL_GetPerformanceCounter();
  return 0;
}
