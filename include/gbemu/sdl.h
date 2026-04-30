#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "gbemu/settings.h"

struct Apu;
struct Ppu;
struct GBemu;

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

extern int win_width, win_height;
extern int menu_bar_height;

extern SDL_Window* win;
extern SDL_Renderer* rnd;

void imgui_init(SDL_Window* window, SDL_Renderer* renderer);
void imgui_shutdown(void);
void imgui_process_event(const SDL_Event* event);

struct AppState {
  struct GBemu* gb;
  char pending_rom[512];
  bool dialog_open;
  bool close_requested;
  char rom_error[512];
  bool settings_open;
  int rebinding_control;
  bool settings_has_draft;
  struct Settings settings_draft;
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
void set_audio_volume(float volume, bool mute);

void take_screenshot(struct Ppu* ppu);
