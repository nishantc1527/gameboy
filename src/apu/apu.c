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
  if (!get_bit(mmu_r_mem(mmu, NR52), 7)) reset_apu();
}

void reset_apu(void) {
  mmu_w_mem(mmu, NR10, 0x00);
  mmu_w_mem(mmu, NR11, 0x00);
  mmu_w_mem(mmu, NR12, 0x00);
  mmu_w_mem(mmu, NR13, 0x00);
  mmu_w_mem(mmu, NR14, 0x00);
  mmu_w_mem(mmu, NR21, 0x00);
  mmu_w_mem(mmu, NR22, 0x00);
  mmu_w_mem(mmu, NR23, 0x00);
  mmu_w_mem(mmu, NR24, 0x00);
  mmu_w_mem(mmu, NR30, 0x00);
  mmu_w_mem(mmu, NR31, 0x00);
  mmu_w_mem(mmu, NR32, 0x00);
  mmu_w_mem(mmu, NR33, 0x00);
  mmu_w_mem(mmu, NR34, 0x00);
  mmu_w_mem(mmu, NR41, 0x00);
  mmu_w_mem(mmu, NR42, 0x00);
  mmu_w_mem(mmu, NR43, 0x00);
  mmu_w_mem(mmu, NR44, 0x00);
  mmu_w_mem(mmu, NR50, 0x00);
  mmu_w_mem(mmu, NR51, 0x00);
  mmu_w_mem(mmu, NR52, 0x00);
}
