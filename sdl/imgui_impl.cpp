#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

extern "C" {
#include "gbemu/core.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
}

#include <string.h>

extern "C" void imgui_init(SDL_Window* window, SDL_Renderer* renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
}

extern "C" void imgui_shutdown(void) {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

extern "C" void imgui_process_event(SDL_Event* event) {
  ImGui_ImplSDL3_ProcessEvent(event);
}


static void draw_menu_bar(struct AppState* state) {
  static bool open_close_modal = false;
  static bool open_quit_modal = false;

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open...", "Ctrl+O")) open_rom_dialog(state);
      if (ImGui::MenuItem("Close", nullptr, false, state->gb != nullptr))
        open_close_modal = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) open_quit_modal = true;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Game", state->gb != nullptr)) {
      bool paused = state->gb->is_paused;
      if (ImGui::MenuItem("Paused", nullptr, &paused))
        state->gb->is_paused = paused;
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (open_close_modal) { ImGui::OpenPopup("Close?"); open_close_modal = false; }
  if (open_quit_modal)  { ImGui::OpenPopup("Quit?");  open_quit_modal  = false; }

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();

  ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Close?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Close the current game?");
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

static void draw_ui_idle(struct AppState* state) {
  ImGuiIO& io = ImGui::GetIO();
  const float pw = 300.0f;
  const float row_h = 32.0f;
  int recent = g_settings.recent_rom_count;
  bool has_error = state->rom_error[0] != '\0';
  float ph = ImGui::GetFrameHeightWithSpacing() +
             ImGui::GetStyle().WindowPadding.y * 2.0f + row_h +
             ImGui::GetStyle().ItemSpacing.y  // open button
             + (has_error ? ImGui::GetTextLineHeightWithSpacing() : 0.0f) +
             (recent > 0 ? ImGui::GetTextLineHeightWithSpacing() + 8.0f +
                               (float)recent *
                                   (row_h + ImGui::GetStyle().ItemSpacing.y)
                         : 0.0f);
  ImGui::SetNextWindowPos(
      ImVec2((io.DisplaySize.x - pw) / 2.0f, (io.DisplaySize.y - ph) / 2.0f),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(pw, ph), ImGuiCond_Always);
  ImGui::Begin("gbemu", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);
  if (ImGui::Button("Open ROM...", ImVec2(-1.0f, row_h)))
    open_rom_dialog(state);
  if (has_error)
    ImGui::TextColored(ImVec4(0.86f, 0.31f, 0.31f, 1.0f), "%s",
                       state->rom_error);
  if (recent > 0) {
    ImGui::Separator();
    ImGui::Text("Recent:");
    for (int i = 0; i < recent; i++) {
      const char* path = g_settings.recent_roms[i];
      const char* name = strrchr(path, '/');
      name = name ? name + 1 : path;
      ImGui::PushID(i);
      if (ImGui::Button(name, ImVec2(-1.0f, row_h)))
        SDL_snprintf(state->pending_rom, sizeof(state->pending_rom), "%s",
                     path);
      ImGui::PopID();
    }
  }
  ImGui::End();
}

extern "C" void draw_ui(struct AppState* state) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  draw_menu_bar(state);
  if (!state->gb) draw_ui_idle(state);
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), rnd);
}
