#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

extern "C" {
#include "gbemu/ppu.h"
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
  SDL_SetRenderLogicalPresentation(rnd, win_width, win_height,
                                   SDL_LOGICAL_PRESENTATION_DISABLED);
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  if (!state->gb) draw_ui_idle(state);
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), rnd);
  SDL_SetRenderLogicalPresentation(rnd, SCRN_WIDTH, SCRN_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
}
