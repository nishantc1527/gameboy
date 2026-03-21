#include <SDL3/SDL.h>
#include <SDL3/SDL_dialog.h>

#include "gbemu/core.h"
#include "gbemu/joypad.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"

static SDL_Gamepad* g_gamepad = NULL;

static void set_btn(struct Ppu* ppu, int btn, int val) { ppu->in[btn] = val; }

static void SDLCALL rom_dialog_callback(void* userdata,
                                        const char* const* filelist,
                                        int filter) {
  (void)filter;
  struct AppState* state = (struct AppState*)userdata;
  state->dialog_open = 0;
  if (filelist && filelist[0])
    SDL_snprintf(state->pending_rom, sizeof(state->pending_rom), "%s",
                 filelist[0]);
}

void open_rom_dialog(struct AppState* state) {
  if (state->dialog_open) return;
  state->dialog_open = 1;
  static const SDL_DialogFileFilter filters[] = {
      {"Game Boy ROMs", "gb;gbc"},
      {"All files", "*"},
  };
  SDL_ShowOpenFileDialog(rom_dialog_callback, state, win, filters, 2, NULL,
                         false);
}

int handle_input(struct AppState* state, SDL_Event* event) {
  nk_sdl_handle_event(ctx, event);

  struct Ppu* ppu = state->gb ? state->gb->ppu : NULL;

  switch (event->type) {
    case SDL_EVENT_QUIT:
      return 1;

    case SDL_EVENT_WINDOW_RESIZED:
      SDL_GetWindowSize(win, &win_width, &win_height);
      break;

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
      if (k == SDLK_F11) {
        SDL_WindowFlags flags = SDL_GetWindowFlags(win);
        SDL_SetWindowFullscreen(win, !(flags & SDL_WINDOW_FULLSCREEN));
      } else if (k == SDLK_O && (event->key.mod & SDL_KMOD_CTRL)) {
        open_rom_dialog(state);
      }
      if (!ppu) break;
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
        state->gb->paused ^= 1;
      else if (k == g_controls[CTRL_SCREENSHOT])
        take_screenshot(ppu);
      else if (k == SDLK_TAB)
        state->gb->fast_forward = 1;
      else if (k == SDLK_R && (event->key.mod & SDL_KMOD_CTRL))
        gbemu_reset(state->gb);
      break;
    }

    case SDL_EVENT_KEY_UP: {
      if (!ppu) break;
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
        state->gb->fast_forward = 0;
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
      if (!ppu) break;
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)
        set_btn(ppu, BTN_A, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)
        set_btn(ppu, BTN_B, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_START)
        set_btn(ppu, BTN_START, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)
        set_btn(ppu, BTN_SELECT, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)
        set_btn(ppu, BTN_UP, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        set_btn(ppu, BTN_DOWN, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)
        set_btn(ppu, BTN_LEFT, 1);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        set_btn(ppu, BTN_RIGHT, 1);
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      if (!ppu) break;
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)
        set_btn(ppu, BTN_A, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)
        set_btn(ppu, BTN_B, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_START)
        set_btn(ppu, BTN_START, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)
        set_btn(ppu, BTN_SELECT, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)
        set_btn(ppu, BTN_UP, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        set_btn(ppu, BTN_DOWN, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)
        set_btn(ppu, BTN_LEFT, 0);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        set_btn(ppu, BTN_RIGHT, 0);
      break;
    }
  }
  return 0;
}
