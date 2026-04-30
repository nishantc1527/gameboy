#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include "gbemu/core.h"
#include "gbemu/joypad.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "sdl_private.h"

static SDL_Gamepad* g_gamepad = NULL;

static void set_key_binding(struct Settings* s, int ctrl, const char* name) {
  switch (ctrl) {
    case CTRL_A:
      SDL_snprintf(s->key_a, 64, "%s", name);
      break;
    case CTRL_B:
      SDL_snprintf(s->key_b, 64, "%s", name);
      break;
    case CTRL_START:
      SDL_snprintf(s->key_start, 64, "%s", name);
      break;
    case CTRL_SELECT:
      SDL_snprintf(s->key_select, 64, "%s", name);
      break;
    case CTRL_UP:
      SDL_snprintf(s->key_up, 64, "%s", name);
      break;
    case CTRL_DOWN:
      SDL_snprintf(s->key_down, 64, "%s", name);
      break;
    case CTRL_LEFT:
      SDL_snprintf(s->key_left, 64, "%s", name);
      break;
    case CTRL_RIGHT:
      SDL_snprintf(s->key_right, 64, "%s", name);
      break;
    case CTRL_PAUSE:
      SDL_snprintf(s->key_pause, 64, "%s", name);
      break;
    case CTRL_SCREENSHOT:
      SDL_snprintf(s->key_screenshot, 64, "%s", name);
      break;
    default:
      break;
  }
}

static void SDLCALL rom_dialog_callback(void* userdata,
                                        const char* const* filelist,
                                        int filter) {
  (void)filter;
  struct AppState* state = (struct AppState*)userdata;
  state->dialog_open = false;
  if (filelist && filelist[0]) {
    SDL_snprintf(state->pending_rom, sizeof(state->pending_rom), "%s",
                 filelist[0]);
  }
}

void open_rom_dialog(struct AppState* state) {
  if (state->dialog_open) {
    return;
  }
  state->dialog_open = true;
  static const SDL_DialogFileFilter filters[] = {
      {"Game Boy ROMs", "gb;gbc"},
      {"All files", "*"},
  };
  SDL_ShowOpenFileDialog(rom_dialog_callback, state, win, filters, 2, NULL,
                         false);
}

int handle_input(struct AppState* state, SDL_Event* event) {
  if (state->rebinding_control >= 0 && event->type == SDL_EVENT_KEY_DOWN) {
    SDL_Keycode k = event->key.key;
    if (k != SDLK_ESCAPE) {
      if (state->settings_has_draft) {
        set_key_binding(&state->settings_draft, state->rebinding_control,
                        SDL_GetKeyName(k));
      } else {
        set_key_binding(&g_settings, state->rebinding_control,
                        SDL_GetKeyName(k));
        populate_controls();
        settings_save(&g_settings);
      }
    }
    state->rebinding_control = -1;
    return 0;
  }
  imgui_process_event(event);

  struct Joypad* joypad = state->gb ? state->gb->joypad : NULL;

  switch (event->type) {
    case SDL_EVENT_QUIT:
      return 1;

    case SDL_EVENT_WINDOW_RESIZED:
      SDL_GetWindowSize(win, &win_width, &win_height);
      break;

    case SDL_EVENT_GAMEPAD_ADDED:
      if (!g_gamepad) {
        g_gamepad = SDL_OpenGamepad(event->gdevice.which);
      }
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
        SDL_SetWindowFullscreen(win, (!(flags & SDL_WINDOW_FULLSCREEN)) != 0);
      } else if (k == SDLK_O && (event->key.mod & SDL_KMOD_CTRL)) {
        open_rom_dialog(state);
      }
      if (!joypad) {
        break;
      }
      if (k == g_controls[CTRL_A]) {
        joypad_set_button(joypad, BTN_A, true, state->gb->bus);
      } else if (k == g_controls[CTRL_B]) {
        joypad_set_button(joypad, BTN_B, true, state->gb->bus);
      } else if (k == g_controls[CTRL_START]) {
        joypad_set_button(joypad, BTN_START, true, state->gb->bus);
      } else if (k == g_controls[CTRL_SELECT]) {
        joypad_set_button(joypad, BTN_SELECT, true, state->gb->bus);
      } else if (k == g_controls[CTRL_UP]) {
        joypad_set_button(joypad, BTN_UP, true, state->gb->bus);
      } else if (k == g_controls[CTRL_DOWN]) {
        joypad_set_button(joypad, BTN_DOWN, true, state->gb->bus);
      } else if (k == g_controls[CTRL_LEFT]) {
        joypad_set_button(joypad, BTN_LEFT, true, state->gb->bus);
      } else if (k == g_controls[CTRL_RIGHT]) {
        joypad_set_button(joypad, BTN_RIGHT, true, state->gb->bus);
      } else if (k == g_controls[CTRL_PAUSE]) {
        state->gb->is_paused = ((!state->gb->is_paused) != 0);
      } else if (k == g_controls[CTRL_SCREENSHOT]) {
        take_screenshot(state->gb->ppu);
      } else if (k == SDLK_TAB) {
        state->gb->fast_forward = true;
      } else if (k == SDLK_R && (event->key.mod & SDL_KMOD_CTRL)) {
        gbemu_reset(state->gb);
      }
      break;
    }

    case SDL_EVENT_KEY_UP: {
      if (!joypad) {
        break;
      }
      SDL_Keycode k = event->key.key;
      if (k == g_controls[CTRL_A]) {
        joypad_set_button(joypad, BTN_A, false, state->gb->bus);
      } else if (k == g_controls[CTRL_B]) {
        joypad_set_button(joypad, BTN_B, false, state->gb->bus);
      } else if (k == g_controls[CTRL_START]) {
        joypad_set_button(joypad, BTN_START, false, state->gb->bus);
      } else if (k == g_controls[CTRL_SELECT]) {
        joypad_set_button(joypad, BTN_SELECT, false, state->gb->bus);
      } else if (k == g_controls[CTRL_UP]) {
        joypad_set_button(joypad, BTN_UP, false, state->gb->bus);
      } else if (k == g_controls[CTRL_DOWN]) {
        joypad_set_button(joypad, BTN_DOWN, false, state->gb->bus);
      } else if (k == g_controls[CTRL_LEFT]) {
        joypad_set_button(joypad, BTN_LEFT, false, state->gb->bus);
      } else if (k == g_controls[CTRL_RIGHT]) {
        joypad_set_button(joypad, BTN_RIGHT, false, state->gb->bus);
      } else if (k == SDLK_TAB) {
        state->gb->fast_forward = false;
      }
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
      if (!joypad) {
        break;
      }
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH) {
        joypad_set_button(joypad, BTN_A, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_EAST) {
        joypad_set_button(joypad, BTN_B, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_START) {
        joypad_set_button(joypad, BTN_START, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_BACK) {
        joypad_set_button(joypad, BTN_SELECT, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP) {
        joypad_set_button(joypad, BTN_UP, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
        joypad_set_button(joypad, BTN_DOWN, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
        joypad_set_button(joypad, BTN_LEFT, true, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
        joypad_set_button(joypad, BTN_RIGHT, true, state->gb->bus);
      }
      break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      if (!joypad) {
        break;
      }
      SDL_GamepadButton btn = (SDL_GamepadButton)event->gbutton.button;
      if (btn == SDL_GAMEPAD_BUTTON_SOUTH) {
        joypad_set_button(joypad, BTN_A, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_EAST) {
        joypad_set_button(joypad, BTN_B, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_START) {
        joypad_set_button(joypad, BTN_START, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_BACK) {
        joypad_set_button(joypad, BTN_SELECT, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP) {
        joypad_set_button(joypad, BTN_UP, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
        joypad_set_button(joypad, BTN_DOWN, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
        joypad_set_button(joypad, BTN_LEFT, false, state->gb->bus);
      } else if (btn == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
        joypad_set_button(joypad, BTN_RIGHT, false, state->gb->bus);
      }
      break;
    }
    default:
      break;
  }
  return 0;
}
