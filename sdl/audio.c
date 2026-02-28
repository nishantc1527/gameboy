#include <SDL3/SDL.h>

#include "gbemu/apu.h"

static SDL_AudioStream* audio_stream = NULL;

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
  SDL_ResumeAudioStreamDevice(audio_stream);
  return 0;
}

void push_audio(struct Apu* apu) {
  if (!audio_stream || apu->sample_count == 0) return;
  int max_queued = APU_SAMPLE_RATE / 10 * 2 * (int)sizeof(float);
  if (SDL_GetAudioStreamQueued(audio_stream) < max_queued) {
    SDL_PutAudioStreamData(audio_stream, apu->sample_buf,
                           (int)(apu->sample_count * 2u * sizeof(float)));
  }
  apu->sample_count = 0;
}
