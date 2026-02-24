#include <SDL3/SDL_audio.h>

SDL_AudioStream* stream;

void init_sdl_apu(void) {
  stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
      &(SDL_AudioSpec){SDL_AUDIO_S32LE, 2, 44100}, NULL, NULL);
}
