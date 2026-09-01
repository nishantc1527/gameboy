#pragma once

#include <SDL3/SDL_render.h>
#include <stdint.h>

#include "gbemu/ppu.h"

enum {
  POKEMON_HEADER_H = 50,
  POKEMON_BOTTOM_H = 70,
  POKEMON_LEFT_W   = 220,
  POKEMON_RIGHT_W  = 260,
};

extern uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];
extern SDL_Texture* txt;

void populate_controls(void);
void pokemon_resize_window(void);
void pokemon_restore_window(void);
