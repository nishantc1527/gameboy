#pragma once

#include <SDL3/SDL.h>

struct Apu;
struct Ppu;
struct gbemu;

enum {
  CTRL_A = 0,
  CTRL_B,
  CTRL_START,
  CTRL_SELECT,
  CTRL_UP,
  CTRL_DOWN,
  CTRL_LEFT,
  CTRL_RIGHT,
  CTRL_PAUSE,
  CTRL_SCREENSHOT,
  CTRL_COUNT
};

extern SDL_Keycode g_controls[CTRL_COUNT];

#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"
#include "nuklear_sdl3_renderer.h"

extern int win_width, win_height;

extern SDL_Window* win;
extern SDL_Renderer* rnd;
extern struct nk_context* ctx;

struct AppState {
  struct gbemu* gb;
  char pending_rom[512];
  int dialog_open;
  char rom_error[512];
};

int init_window(const char* title);
void set_window_title_rom(const char* rom_title);

int handle_input(struct AppState* state, SDL_Event* event);
void open_rom_dialog(struct AppState* state);
void render(struct Ppu* ppu);
void draw_ui(struct AppState* state);

int init_audio(void);
int audio_queued_bytes(void);
void push_audio(struct Apu* apu);

void take_screenshot(struct Ppu* ppu);
