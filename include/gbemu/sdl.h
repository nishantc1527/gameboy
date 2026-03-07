#pragma once

#include <SDL3/SDL.h>

#include "gbemu/gbemu.h"
#include "gbemu/ppu.h"
#include "gbemu/settings.h"

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

extern Uint64 tot_ticks;
extern Uint64 prev_time;
extern Uint64 perf_freq;

typedef struct {
  gbemu* gb;
  char pending_rom[512];
  int dialog_open;
} AppState;

int init_window(const char* title);
void set_window_title_rom(const char* rom_title);

int handle_input(AppState* state, SDL_Event* event);
void open_rom_dialog(AppState* state);
void render(struct Ppu* ppu);
void draw_ui(AppState* state);

struct Apu;
int init_audio(void);
void push_audio(struct Apu* apu);
