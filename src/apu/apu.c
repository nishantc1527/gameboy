#include "gbemu/apu.h"

#include <SDL3/SDL_audio.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"

SDL_AudioStream* stream;
uint8_t ch1, ch2, ch3, ch4;

void init_apu(void) {
  stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
      &(SDL_AudioSpec){SDL_AUDIO_S32LE, 2, 44100}, NULL, NULL);
  ch1 = ch2 = ch3 = ch4 = 0;
}

void upd_apu(void) {
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (ch1) set_bit(&ctrl, 0);
  if (ch2) set_bit(&ctrl, 1);
  if (ch3) set_bit(&ctrl, 2);
  if (ch4) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
}
