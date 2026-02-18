#pragma once

#include <SDL3/SDL_audio.h>
#include <stdint.h>

extern SDL_AudioStream* stream;
extern uint8_t ch1_enable, ch2_enable, ch3_enable, ch4_enable;
extern uint8_t ch1_len_enable, ch1_len, ch1_len_apu_div, ch1_len_init,
    ch1_trigger;
extern uint8_t div_apu;

void init_apu(void);
void upd_apu(void);

void reset_apu(void);
