#pragma once

#include <SDL3/SDL_audio.h>
#include <stdint.h>

#include "mmu.h"

extern SDL_AudioStream* stream;
extern uint8_t ch1, ch2, ch3, ch4;

void init_apu(void);
void upd_apu(void);

void reset_apu(void);
