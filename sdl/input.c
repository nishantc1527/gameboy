#include <SDL3/SDL.h>

#include "gbemu/ppu.h"
#include "gbemu/sdl.h"

int handle_input(struct Ppu* ppu, SDL_Event* event) {
  SDL_ConvertEventToRenderCoordinates(rnd, event);
  nk_sdl_handle_event(ctx, event);
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return 1;
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A]) ppu->in[BTN_A] = 1;
      else if (k == g_controls[CTRL_B]) ppu->in[BTN_B] = 1;
      else if (k == g_controls[CTRL_START]) ppu->in[BTN_START] = 1;
      else if (k == g_controls[CTRL_SELECT]) ppu->in[BTN_SELECT] = 1;
      else if (k == g_controls[CTRL_UP]) ppu->in[BTN_UP] = 1;
      else if (k == g_controls[CTRL_DOWN]) ppu->in[BTN_DOWN] = 1;
      else if (k == g_controls[CTRL_LEFT]) ppu->in[BTN_LEFT] = 1;
      else if (k == g_controls[CTRL_RIGHT]) ppu->in[BTN_RIGHT] = 1;
      break;
    }
    case SDL_EVENT_KEY_UP: {
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A]) ppu->in[BTN_A] = 0;
      else if (k == g_controls[CTRL_B]) ppu->in[BTN_B] = 0;
      else if (k == g_controls[CTRL_START]) ppu->in[BTN_START] = 0;
      else if (k == g_controls[CTRL_SELECT]) ppu->in[BTN_SELECT] = 0;
      else if (k == g_controls[CTRL_UP]) ppu->in[BTN_UP] = 0;
      else if (k == g_controls[CTRL_DOWN]) ppu->in[BTN_DOWN] = 0;
      else if (k == g_controls[CTRL_LEFT]) ppu->in[BTN_LEFT] = 0;
      else if (k == g_controls[CTRL_RIGHT]) ppu->in[BTN_RIGHT] = 0;
      break;
    }
  }
  return 0;
}
