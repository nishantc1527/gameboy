#include "gbemu/apu.h"

#include <SDL3/SDL_audio.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"

SDL_AudioStream* stream;
uint8_t ch1_enable, ch2_enable, ch3_enable, ch4_enable;
uint8_t ch1_len_enable, ch1_len, ch1_len_apu_div, ch1_len_init, ch1_trigger;
uint8_t div_apu;

void init_apu(void) {
  stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
      &(SDL_AudioSpec){SDL_AUDIO_S32LE, 2, 44100}, NULL, NULL);
  ch1_enable = ch2_enable = ch3_enable = ch4_enable = 0;
  ch1_len_enable = ch1_trigger = 0;
  ch1_len = ch1_len_apu_div = ch1_len_init = 0x00;
  div_apu = 0x00;
  reset_apu();
}

void upd_apu(void) {
  if (!get_bit(mmu_r_mem(mmu, NR52), 7)) {
    reset_apu();
    return;
  }
  uint8_t nr14 = mmu_r_mem_raw(mmu, NR14);
  uint8_t curr_trigger = get_bit(nr14, 7);
  if (curr_trigger && !ch1_trigger) {
    ch1_trigger = 1;
    ch1_enable = 1;
    if (get_bit(nr14, 6)) {
      ch1_len_enable = 1;
      uint8_t nr11 = mmu_r_mem_raw(mmu, NR11);
      if (nr11 != ch1_len_init) {
        ch1_len = nr11 & 0b111111;
        ch1_len_apu_div = div_apu;
      }
    }
  } else if (!curr_trigger && ch1_trigger) {
    ch1_trigger = 0;
    ch1_enable = 0;
  }
  if (ch1_enable && ch1_len_enable &&
      (uint8_t)(div_apu - ch1_len_apu_div) >= 2) {
    ch1_len++;
    ch1_len_apu_div = div_apu;
    if (ch1_len == 0x40) ch1_enable = 0;
  }
  uint8_t ctrl = mmu_r_mem(mmu, NR52) & 0xF0;
  if (ch1_enable) set_bit(&ctrl, 0);
  if (ch2_enable) set_bit(&ctrl, 1);
  if (ch3_enable) set_bit(&ctrl, 2);
  if (ch4_enable) set_bit(&ctrl, 3);
  mmu_w_mem(mmu, NR52, ctrl);
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
