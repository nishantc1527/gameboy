#pragma once

#include <SDL3/SDL_render.h>
#include <stdint.h>

#include "gbemu/ppu.h"

extern uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];
extern SDL_Texture* txt;

void populate_controls(void);
