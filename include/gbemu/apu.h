#pragma once

#include <SDL3/SDL_audio.h>
#include <stdint.h>

extern SDL_AudioStream* stream;
extern uint8_t div_apu;

void init_apu(void);
void upd_apu(void);
void reset_apu(void);
