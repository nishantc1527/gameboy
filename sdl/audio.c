#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "gbemu/apu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"

static SDL_AudioStream* audio_stream = nullptr;

int init_audio(void) {
  SDL_AudioSpec spec = {
      .format = SDL_AUDIO_F32,
      .channels = 2,
      .freq = APU_SAMPLE_RATE,
  };
  audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &spec, NULL, NULL);
  if (!audio_stream) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio device: %s\n",
                 SDL_GetError());
    return 1;
  }
  SDL_SetAudioStreamGain(audio_stream,
                         g_settings.mute ? 0.0f : g_settings.volume);
  SDL_ResumeAudioStreamDevice(audio_stream);
  float silence[APU_BUF_SIZE * 2] = {0};
  SDL_PutAudioStreamData(audio_stream, silence, (int)sizeof(silence));
  return 0;
}

void set_audio_volume(float volume, bool mute) {
  if (audio_stream) SDL_SetAudioStreamGain(audio_stream, mute ? 0.0f : volume);
}

int audio_queued_bytes(void) {
  if (!audio_stream) {
    return 0;
  }
  return SDL_GetAudioStreamQueued(audio_stream);
}

void push_audio(struct Apu* apu) {
  if (!audio_stream || apu_get_sample_count(apu) == 0) {
    return;
  }
  SDL_PutAudioStreamData(
      audio_stream, apu_get_sample_buf(apu),
      (int)((size_t)apu_get_sample_count(apu) * 2U * sizeof(float)));
  apu_discard_samples(apu);
}
