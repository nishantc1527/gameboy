#pragma once

#include <SDL3/SDL.h>

#include "gbemu/ppu.h"

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

int init_window(const char* rom_title, uint8_t headless);

int handle_input(struct PPU* ppu, SDL_Event* event, uint8_t headless);
void render(struct PPU* ppu);
void draw_ui(void);
