#include <SDL3/SDL.h>

#include "gbemu/gbemu.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"

static SDL_Gamepad* g_gamepad = NULL;

static void set_btn(struct Ppu* ppu, int btn, int val) { ppu->in[btn] = val; }

int handle_input(gbemu* gb, SDL_Event* event) {
  struct Ppu* ppu = gb->ppu;
  SDL_ConvertEventToRenderCoordinates(rnd, event);
  nk_sdl_handle_event(ctx, event);
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return 1;

    case SDL_EVENT_GAMEPAD_ADDED:
      if (!g_gamepad) g_gamepad = SDL_OpenGamepad(event->gdevice.which);
      break;

    case SDL_EVENT_GAMEPAD_REMOVED:
      if (g_gamepad && SDL_GetGamepadID(g_gamepad) == event->gdevice.which) {
        SDL_CloseGamepad(g_gamepad);
        g_gamepad = NULL;
      }
      break;

    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A])
        set_btn(ppu, BTN_A, 1);
      else if (k == g_controls[CTRL_B])
        set_btn(ppu, BTN_B, 1);
      else if (k == g_controls[CTRL_START])
        set_btn(ppu, BTN_START, 1);
      else if (k == g_controls[CTRL_SELECT])
        set_btn(ppu, BTN_SELECT, 1);
      else if (k == g_controls[CTRL_UP])
        set_btn(ppu, BTN_UP, 1);
      else if (k == g_controls[CTRL_DOWN])
        set_btn(ppu, BTN_DOWN, 1);
      else if (k == g_controls[CTRL_LEFT])
        set_btn(ppu, BTN_LEFT, 1);
      else if (k == g_controls[CTRL_RIGHT])
        set_btn(ppu, BTN_RIGHT, 1);
      else if (k == g_controls[CTRL_PAUSE])
        gb->paused ^= 1;
      else if (k == SDLK_TAB)
        gb->fast_forward = 1;
      else if (k == SDLK_F11) {
        SDL_WindowFlags flags = SDL_GetWindowFlags(win);
        SDL_SetWindowFullscreen(win, !(flags & SDL_WINDOW_FULLSCREEN));
      } else if (k == SDLK_R && (event->key.mod & SDL_KMOD_CTRL)) {
        gbemu_reset(gb);
      }
      break;
    }

    case SDL_EVENT_KEY_UP: {
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A])
        set_btn(ppu, BTN_A, 0);
      else if (k == g_controls[CTRL_B])
        set_btn(ppu, BTN_B, 0);
      else if (k == g_controls[CTRL_START])
        set_btn(ppu, BTN_START, 0);
      else if (k == g_controls[CTRL_SELECT])
        set_btn(ppu, BTN_SELECT, 0);
      else if (k == g_controls[CTRL_UP])
        set_btn(ppu, BTN_UP, 0);
      else if (k == g_controls[CTRL_DOWN])
        set_btn(ppu, BTN_DOWN, 0);
      else if (k == g_controls[CTRL_LEFT])
        set_btn(ppu, BTN_LEFT, 0);
      else if (k == g_controls[CTRL_RIGHT])
        set_btn(ppu, BTN_RIGHT, 0);
      else if (k == SDLK_TAB)
        gb->fast_forward = 0;
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)           set_btn(ppu, BTN_A, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)       set_btn(ppu, BTN_B, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_START)      set_btn(ppu, BTN_START, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)       set_btn(ppu, BTN_SELECT, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)    set_btn(ppu, BTN_UP, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)  set_btn(ppu, BTN_DOWN, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)  set_btn(ppu, BTN_LEFT, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) set_btn(ppu, BTN_RIGHT, 1);
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)           set_btn(ppu, BTN_A, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)       set_btn(ppu, BTN_B, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_START)      set_btn(ppu, BTN_START, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)       set_btn(ppu, BTN_SELECT, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)    set_btn(ppu, BTN_UP, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)  set_btn(ppu, BTN_DOWN, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)  set_btn(ppu, BTN_LEFT, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) set_btn(ppu, BTN_RIGHT, 0);
      break;
    }
  }
  return 0;
}
