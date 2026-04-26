#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>

#include "gbemu/core.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "sdl_private.h"

// TODO sdl state
int win_width, win_height;
int menu_bar_height = 19;
SDL_Texture* txt = NULL;

SDL_Window* win;
SDL_Renderer* rnd;

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
  if (!SDL_CreateWindowAndRenderer(
          rom_title, SCRN_WIDTH * g_settings.scale,
          SCRN_HEIGHT * g_settings.scale + menu_bar_height, 0, &win, &rnd)) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                 "ERROR CREATING WINDOW & RENDERER: %s\n", SDL_GetError());
    return 1;
  }
  if (g_settings.fullscreen) SDL_SetWindowFullscreen(win, true);
  SDL_SetRenderVSync(rnd, 0);
  SDL_GetWindowSize(win, &win_width, &win_height);
  txt = SDL_CreateTexture(rnd, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
  SDL_SetTextureScaleMode(txt, SDL_SCALEMODE_NEAREST);
  if (!txt) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNABLE TO CREATE TEXTURE\n");
    return 0;
  }
  imgui_init(win, rnd);
  init_audio();
  return 0;
}

void set_window_title_rom(const char* rom_title) {
  char buf[160];
  SDL_snprintf(buf, sizeof(buf), "gbemu " GBEMU_VERSION " \xe2\x80\x94 %s",
               rom_title);
  SDL_SetWindowTitle(win, buf);
}
