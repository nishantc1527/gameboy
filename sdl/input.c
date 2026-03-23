#include <SDL3/SDL.h>
#include <SDL3/SDL_dialog.h>

#include "gbemu/core.h"
#include "gbemu/joypad.h"
#include "gbemu/sdl.h"

static SDL_Gamepad* g_gamepad = NULL;

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

  struct Joypad* joypad = state->gb ? state->gb->joypad : NULL;

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
      if (!joypad) break;
      if (k == g_controls[CTRL_A])
        joypad_set_button(joypad, BTN_A, true, state->gb->bus);
      else if (k == g_controls[CTRL_B])
        joypad_set_button(joypad, BTN_B, true, state->gb->bus);
      else if (k == g_controls[CTRL_START])
        joypad_set_button(joypad, BTN_START, true, state->gb->bus);
      else if (k == g_controls[CTRL_SELECT])
        joypad_set_button(joypad, BTN_SELECT, true, state->gb->bus);
      else if (k == g_controls[CTRL_UP])
        joypad_set_button(joypad, BTN_UP, true, state->gb->bus);
      else if (k == g_controls[CTRL_DOWN])
        joypad_set_button(joypad, BTN_DOWN, true, state->gb->bus);
      else if (k == g_controls[CTRL_LEFT])
        joypad_set_button(joypad, BTN_LEFT, true, state->gb->bus);
      else if (k == g_controls[CTRL_RIGHT])
        joypad_set_button(joypad, BTN_RIGHT, true, state->gb->bus);
      else if (k == g_controls[CTRL_PAUSE])
        state->gb->paused ^= 1;
      else if (k == g_controls[CTRL_SCREENSHOT])
        take_screenshot(state->gb->ppu);
      else if (k == SDLK_TAB)
        state->gb->fast_forward = 1;
      else if (k == SDLK_R && (event->key.mod & SDL_KMOD_CTRL))
        gbemu_reset(state->gb);
      break;
    }

    case SDL_EVENT_KEY_UP: {
      if (!joypad) break;
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A])
        joypad_set_button(joypad, BTN_A, false, state->gb->bus);
      else if (k == g_controls[CTRL_B])
        joypad_set_button(joypad, BTN_B, false, state->gb->bus);
      else if (k == g_controls[CTRL_START])
        joypad_set_button(joypad, BTN_START, false, state->gb->bus);
      else if (k == g_controls[CTRL_SELECT])
        joypad_set_button(joypad, BTN_SELECT, false, state->gb->bus);
      else if (k == g_controls[CTRL_UP])
        joypad_set_button(joypad, BTN_UP, false, state->gb->bus);
      else if (k == g_controls[CTRL_DOWN])
        joypad_set_button(joypad, BTN_DOWN, false, state->gb->bus);
      else if (k == g_controls[CTRL_LEFT])
        joypad_set_button(joypad, BTN_LEFT, false, state->gb->bus);
      else if (k == g_controls[CTRL_RIGHT])
        joypad_set_button(joypad, BTN_RIGHT, false, state->gb->bus);
      else if (k == SDLK_TAB)
        state->gb->fast_forward = 0;
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
      if (!joypad) break;
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)
        joypad_set_button(joypad, BTN_A, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)
        joypad_set_button(joypad, BTN_B, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_START)
        joypad_set_button(joypad, BTN_START, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)
        joypad_set_button(joypad, BTN_SELECT, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)
        joypad_set_button(joypad, BTN_UP, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        joypad_set_button(joypad, BTN_DOWN, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)
        joypad_set_button(joypad, BTN_LEFT, true, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        joypad_set_button(joypad, BTN_RIGHT, true, state->gb->bus);
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      if (!joypad) break;
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH)
        joypad_set_button(joypad, BTN_A, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_EAST)
        joypad_set_button(joypad, BTN_B, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_START)
        joypad_set_button(joypad, BTN_START, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_BACK)
        joypad_set_button(joypad, BTN_SELECT, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)
        joypad_set_button(joypad, BTN_UP, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        joypad_set_button(joypad, BTN_DOWN, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT)
        joypad_set_button(joypad, BTN_LEFT, false, state->gb->bus);
      else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        joypad_set_button(joypad, BTN_RIGHT, false, state->gb->bus);
      break;
    }
  }
  return 0;
}
