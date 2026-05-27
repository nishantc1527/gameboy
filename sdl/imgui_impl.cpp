#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

extern "C" {
#include "gbemu/core.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "sdl_private.h"
}

#include <cstring>

extern "C" void imgui_init(SDL_Window* window, SDL_Renderer* renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 4.0f;
  style.FrameRounding  = 3.0f;
  style.PopupRounding  = 4.0f;
  style.GrabRounding   = 3.0f;
  style.FramePadding   = ImVec2(6.0f, 4.0f);
  style.ItemSpacing    = ImVec2(8.0f, 6.0f);
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
}

extern "C" void imgui_shutdown(void) {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

extern "C" void imgui_process_event(const SDL_Event* event) {
  ImGui_ImplSDL3_ProcessEvent(event);
}

static ImVec4 u32_to_imvec4(uint32_t c) {
  return { ((c >> 16) & 0xFF) / 255.0f,
           ((c >>  8) & 0xFF) / 255.0f,
           ( c        & 0xFF) / 255.0f, 1.0f };
}

static uint32_t imvec4_to_u32(const ImVec4& c) {
  return ((uint32_t)(c.x * 255.0f + 0.5f) << 16) |
         ((uint32_t)(c.y * 255.0f + 0.5f) <<  8) |
          (uint32_t)(c.z * 255.0f + 0.5f);
}

static char* key_field(struct Settings* s, int i) {
  switch (i) {
    case CTRL_A:          return s->key_a;
    case CTRL_B:          return s->key_b;
    case CTRL_START:      return s->key_start;
    case CTRL_SELECT:     return s->key_select;
    case CTRL_UP:         return s->key_up;
    case CTRL_DOWN:       return s->key_down;
    case CTRL_LEFT:       return s->key_left;
    case CTRL_RIGHT:      return s->key_right;
    case CTRL_PAUSE:      return s->key_pause;
    case CTRL_SCREENSHOT: return s->key_screenshot;
    default:              return nullptr;
  }
}

static bool settings_differ(const struct Settings* a, const struct Settings* b) {
  if (a->scale != b->scale || a->fullscreen != b->fullscreen) return true;
  if (a->mute != b->mute || a->volume != b->volume) return true;
  for (int i = 0; i < 4; i++)
    if (a->dmg_palette[i] != b->dmg_palette[i]) return true;
  if (strcmp(a->key_a,          b->key_a))          return true;
  if (strcmp(a->key_b,          b->key_b))          return true;
  if (strcmp(a->key_start,      b->key_start))      return true;
  if (strcmp(a->key_select,     b->key_select))     return true;
  if (strcmp(a->key_up,         b->key_up))         return true;
  if (strcmp(a->key_down,       b->key_down))       return true;
  if (strcmp(a->key_left,       b->key_left))       return true;
  if (strcmp(a->key_right,      b->key_right))      return true;
  if (strcmp(a->key_pause,      b->key_pause))      return true;
  if (strcmp(a->key_screenshot, b->key_screenshot)) return true;
  return false;
}

static void apply_settings(AppState* state) {
  struct Settings* d = &state->settings_draft;
  if (d->scale != g_settings.scale)
    SDL_SetWindowSize(win, SCRN_WIDTH * d->scale, SCRN_HEIGHT * d->scale + menu_bar_height);
  if (d->fullscreen != g_settings.fullscreen)
    SDL_SetWindowFullscreen(win, d->fullscreen);
  struct Settings next = *d;
  memcpy(next.last_rom,    g_settings.last_rom,    sizeof(next.last_rom));
  memcpy(next.recent_roms, g_settings.recent_roms, sizeof(next.recent_roms));
  next.recent_rom_count = g_settings.recent_rom_count;
  g_settings = next;
  set_audio_volume(g_settings.volume, g_settings.mute);
  populate_controls();
  settings_save(&g_settings);
}

static const char* ctrl_names[CTRL_COUNT] = {
  "A", "B", "Start", "Select", "Up", "Down", "Left", "Right", "Pause", "Screenshot"
};

static void draw_settings_window(AppState* state) {
  if (!state->settings_has_draft) {
    state->settings_draft    = g_settings;
    state->settings_has_draft = true;
  }
  struct Settings* d = &state->settings_draft;

  const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Once);
  bool open = true;
  if (ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize)) {
    if (ImGui::BeginTabBar("##tabs")) {

      if (ImGui::BeginTabItem("Display")) {
        ImGui::SliderInt("Scale", &d->scale, 1, 5);
        ImGui::Checkbox("Fullscreen", &d->fullscreen);
        ImGui::Spacing();
        ImGui::TextDisabled("DMG Palette");
        static const char* pal_labels[4] = {
          "Color 0 (lightest)", "Color 1", "Color 2", "Color 3 (darkest)"
        };
        for (int i = 0; i < 4; i++) {
          ImVec4 col = u32_to_imvec4(d->dmg_palette[i]);
          ImGui::PushID(i);
          if (ImGui::ColorEdit3(pal_labels[i], &col.x, ImGuiColorEditFlags_NoAlpha))
            d->dmg_palette[i] = imvec4_to_u32(col);
          ImGui::PopID();
        }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Audio")) {
        ImGui::SliderFloat("Volume", &d->volume, 0.0f, 1.0f, "%.2f");
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Controls")) {
        ImGui::TextDisabled("Click a binding to rebind. Escape cancels.");
        ImGui::Spacing();
        if (ImGui::BeginTable("##controls", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
          ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthFixed, 100.0f);
          ImGui::TableSetupColumn("Key",     ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableHeadersRow();
          for (int i = 0; i < CTRL_COUNT; i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ctrl_names[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if (state->rebinding_control == i) {
              ImGui::BeginDisabled();
              ImGui::Button("[Press any key…]", ImVec2(-1.0f, 0.0f));
              ImGui::EndDisabled();
            } else {
              const char* kf = key_field(d, i);
              const char* label = (kf && kf[0]) ? kf : "???";
              if (ImGui::Button(label, ImVec2(-1.0f, 0.0f)))
                state->rebinding_control = i;
            }
            ImGui::PopID();
          }
          ImGui::EndTable();
        }
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    bool no_changes = !settings_differ(d, &g_settings);
    ImGui::BeginDisabled(no_changes);
    if (ImGui::Button("Apply")) {
      apply_settings(state);
      state->settings_draft = g_settings;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    static bool confirm_reset = false;
    if (ImGui::Button("Reset to Defaults")) confirm_reset = true;
    if (confirm_reset) { ImGui::OpenPopup("##settings_reset"); confirm_reset = false; }
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("##settings_reset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Reset all settings to defaults?");
      ImGui::Spacing();
      if (ImGui::Button("Reset", ImVec2(80, 0))) {
        settings_defaults(d);
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(80, 0))) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }
  ImGui::End();

  static bool confirm_close = false;
  if (!open) {
    if (settings_differ(d, &g_settings)) {
      confirm_close = true;
    } else {
      state->settings_has_draft = false;
      state->settings_open      = false;
    }
  }

  if (confirm_close) { ImGui::OpenPopup("##settings_close"); confirm_close = false; }
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("##settings_close", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Save changes before closing?");
    ImGui::Spacing();
    if (ImGui::Button("Save", ImVec2(80, 0))) {
      apply_settings(state);
      state->settings_has_draft = false;
      state->settings_open      = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard", ImVec2(80, 0))) {
      state->settings_has_draft = false;
      state->settings_open      = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

static void draw_menu_bar(AppState* state) {
  static bool open_close_modal = false;
  static bool open_quit_modal  = false;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open...", "Ctrl+O")) open_rom_dialog(state);
      if (ImGui::MenuItem("Close", nullptr, false, state->gb != nullptr))
        open_close_modal = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Settings")) state->settings_open = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) open_quit_modal = true;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Game", state->gb != nullptr)) {
      bool paused = state->gb->is_paused;
      if (ImGui::MenuItem("Pause", nullptr, &paused))
        state->gb->is_paused = paused;
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar();
  if (open_close_modal) { ImGui::OpenPopup("Close?"); open_close_modal = false; }
  if (open_quit_modal)  { ImGui::OpenPopup("Quit?");  open_quit_modal  = false; }
  const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Close?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Close the current game? Any unsaved changes will be lost.");
    ImGui::Spacing();
    if (ImGui::Button("Yes", ImVec2(80, 0))) {
      state->close_requested = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("No", ImVec2(80, 0))) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Quit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Quit gbemu?");
    ImGui::Spacing();
    if (ImGui::Button("Yes", ImVec2(80, 0))) {
      SDL_Event e = {};
      e.type = SDL_EVENT_QUIT;
      SDL_PushEvent(&e);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("No", ImVec2(80, 0))) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

static void draw_ui_idle(AppState* state) {
  constexpr float pw    = 280.0f;
  constexpr float row_h = 28.0f;
  const int   recent    = g_settings.recent_rom_count;
  const bool  has_error = state->rom_error[0] != '\0';
  const ImGuiStyle& style = ImGui::GetStyle();
  float ph = style.WindowPadding.y * 2.0f
           + ImGui::GetTextLineHeightWithSpacing()
           + 1.0f + style.ItemSpacing.y * 2.0f
           + row_h + style.ItemSpacing.y;
  if (has_error) ph += ImGui::GetTextLineHeightWithSpacing();
  if (recent > 0) {
    ph += 1.0f + style.ItemSpacing.y * 2.0f
        + ImGui::GetTextLineHeightWithSpacing()
        + (float)recent * (row_h + style.ItemSpacing.y);
  }
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(pw, ph), ImGuiCond_Always);
  ImGui::Begin("##idle", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
  const char* title   = "gbemu";
  float       title_w = ImGui::CalcTextSize(title).x;
  ImGui::SetCursorPosX((pw - title_w) / 2.0f);
  ImGui::TextDisabled("%s", title);
  ImGui::Separator();
  if (ImGui::Button("Open ROM...", ImVec2(-1.0f, row_h))) open_rom_dialog(state);
  if (has_error)
    ImGui::TextColored(ImVec4(0.86f, 0.31f, 0.31f, 1.0f), "%s", state->rom_error);
  if (recent > 0) {
    ImGui::Separator();
    ImGui::TextDisabled("Recent");
    for (int i = 0; i < recent; i++) {
      const char* path = g_settings.recent_roms[i];
      const char* name = strrchr(path, '/');
      name = name ? name + 1 : path;
      ImGui::PushID(i);
      if (ImGui::Button(name, ImVec2(-1.0f, row_h)))
        SDL_snprintf(state->pending_rom, sizeof(state->pending_rom), "%s", path);
      ImGui::PopID();
    }
  }
  ImGui::End();
}

extern "C" void draw_ui(AppState* state) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  draw_menu_bar(state);
  if (!state->gb) draw_ui_idle(state);
  if (state->settings_open) draw_settings_window(state);
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), rnd);
}