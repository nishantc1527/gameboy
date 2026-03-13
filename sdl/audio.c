#include <SDL3/SDL.h>

#include "gbemu/apu.h"
#include "gbemu/settings.h"

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
  SDL_SetAudioStreamGain(audio_stream, g_settings.volume);
  SDL_ResumeAudioStreamDevice(audio_stream);
  float silence[APU_BUF_SIZE * 2] = {0};
  SDL_PutAudioStreamData(audio_stream, silence, (int)sizeof(silence));
  return 0;
}

int audio_queued_bytes(void) {
  if (!audio_stream) return 0;
  return (int)SDL_GetAudioStreamQueued(audio_stream);
}

void push_audio(struct Apu* apu) {
  if (!audio_stream || apu->sample_count == 0) return;
  SDL_PutAudioStreamData(audio_stream, apu->sample_buf,
                         (int)(apu->sample_count * 2u * sizeof(float)));
  apu->sample_count = 0;
}
