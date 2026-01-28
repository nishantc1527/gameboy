#pragma once

#include <SDL3/SDL.h>

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

int init_window(void);

int handle_input(SDL_Event* event);
void render(void);
void draw_ui(void);
