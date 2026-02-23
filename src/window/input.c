#include <SDL3/SDL.h>

#include "controls.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"

int handle_input(struct PPU* ppu, SDL_Event* event, uint8_t headless) {
  SDL_ConvertEventToRenderCoordinates(rnd, event);
  if (!headless) nk_sdl_handle_event(ctx, event);
  switch (event->type) {
    case SDL_EVENT_QUIT: return 1;
    case SDL_EVENT_KEY_DOWN:
      switch (event->key.key) {
        case KEY_A: ppu->in[BTN_A] = 1; break;
        case KEY_B: ppu->in[BTN_B] = 1; break;
        case KEY_START: ppu->in[BTN_START] = 1; break;
        case KEY_SELECT: ppu->in[BTN_SELECT] = 1; break;
        case KEY_UP: ppu->in[BTN_UP] = 1; break;
        case KEY_DOWN: ppu->in[BTN_DOWN] = 1; break;
        case KEY_LEFT: ppu->in[BTN_LEFT] = 1; break;
        case KEY_RIGHT: ppu->in[BTN_RIGHT] = 1; break;
      }
      break;
    case SDL_EVENT_KEY_UP:
      switch (event->key.key) {
        case KEY_A: ppu->in[BTN_A] = 0; break;
        case KEY_B: ppu->in[BTN_B] = 0; break;
        case KEY_START: ppu->in[BTN_START] = 0; break;
        case KEY_SELECT: ppu->in[BTN_SELECT] = 0; break;
        case KEY_UP: ppu->in[BTN_UP] = 0; break;
        case KEY_DOWN: ppu->in[BTN_DOWN] = 0; break;
        case KEY_LEFT: ppu->in[BTN_LEFT] = 0; break;
        case KEY_RIGHT: ppu->in[BTN_RIGHT] = 0; break;
      }
      break;
  }
  return 0;
}
