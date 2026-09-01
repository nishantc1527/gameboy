#include "imgui.h"

extern "C" {
#include "gbemu/core.h"
#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "sdl_private.h"
}

#include <cstdio>
#include <cstring>

static const ImVec4 PKM_TYPE_COLOR[27] = {
  /* 0x00 NORMAL   */ {0.659f, 0.659f, 0.471f, 1.0f},
  /* 0x01 FIGHTING */ {0.750f, 0.188f, 0.157f, 1.0f},
  /* 0x02 FLYING   */ {0.659f, 0.565f, 0.941f, 1.0f},
  /* 0x03 POISON   */ {0.627f, 0.251f, 0.627f, 1.0f},
  /* 0x04 GROUND   */ {0.878f, 0.753f, 0.408f, 1.0f},
  /* 0x05 ROCK     */ {0.722f, 0.627f, 0.220f, 1.0f},
  /* 0x06 BIRD     */ {0.659f, 0.659f, 0.471f, 1.0f},
  /* 0x07 BUG      */ {0.659f, 0.722f, 0.125f, 1.0f},
  /* 0x08 GHOST    */ {0.439f, 0.345f, 0.596f, 1.0f},
  /* 0x09–0x13 unused */ {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
  /* 0x14 FIRE     */ {0.941f, 0.502f, 0.188f, 1.0f},
  /* 0x15 WATER    */ {0.408f, 0.565f, 0.941f, 1.0f},
  /* 0x16 GRASS    */ {0.471f, 0.784f, 0.314f, 1.0f},
  /* 0x17 ELECTRIC */ {0.973f, 0.816f, 0.188f, 1.0f},
  /* 0x18 PSYCHIC  */ {0.973f, 0.345f, 0.533f, 1.0f},
  /* 0x19 ICE      */ {0.596f, 0.847f, 0.847f, 1.0f},
  /* 0x1A DRAGON   */ {0.439f, 0.220f, 0.973f, 1.0f},
};

static ImVec4 type_color(uint8_t type_id) {
  if (type_id < 27) return PKM_TYPE_COLOR[type_id];
  return PKM_TYPE_COLOR[0];
}

static const char* type_name(uint8_t type_id) {
  if (type_id < 27 && pokemon_type_names[type_id])
    return pokemon_type_names[type_id];
  return "???";
}

static const char* species_name(uint8_t id) {
  if (id > 0 && pokemon_species_names[id]) return pokemon_species_names[id];
  return "???";
}

static void draw_type_badge(uint8_t type_id) {
  ImVec4 col = type_color(type_id);
  ImGui::PushStyleColor(ImGuiCol_Button, col);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  col);
  ImGui::SmallButton(type_name(type_id));
  ImGui::PopStyleColor(3);
}

static void draw_hp_bar(float frac, float width) {
  if (frac < 0.0f) frac = 0.0f;
  if (frac > 1.0f) frac = 1.0f;
  ImVec4 col = frac > 0.50f ? ImVec4(0.18f, 0.75f, 0.25f, 1.0f)
             : frac > 0.25f ? ImVec4(0.93f, 0.79f, 0.06f, 1.0f)
                            : ImVec4(0.86f, 0.20f, 0.18f, 1.0f);
  ImVec2 pos  = ImGui::GetCursorScreenPos();
  float  h    = ImGui::GetTextLineHeight() * 0.55f;
  ImDrawList* dl = ImGui::GetWindowDrawList();
  dl->AddRectFilled(pos, {pos.x + width, pos.y + h},
                    IM_COL32(40, 40, 40, 200), 2.0f);
  if (frac > 0.0f) {
    ImVec4 cf = col;
    dl->AddRectFilled(pos, {pos.x + width * frac, pos.y + h},
                      ImGui::ColorConvertFloat4ToU32(cf), 2.0f);
  }
  ImGui::Dummy({width, h});
}

static ImVec4 status_color(uint8_t status, uint16_t hp) {
  if (hp == 0)       return {0.50f, 0.50f, 0.50f, 1.0f};
  if (status & 0x40) return {1.00f, 0.90f, 0.10f, 1.0f};
  if (status & 0x20) return {0.40f, 0.90f, 0.95f, 1.0f};
  if (status & 0x10) return {1.00f, 0.45f, 0.05f, 1.0f};
  if (status & 0x08) return {0.75f, 0.20f, 0.90f, 1.0f};
  if (status & 0x07) return {0.50f, 0.50f, 0.50f, 1.0f};
  return {1.0f, 1.0f, 1.0f, 1.0f};
}

static void draw_pokemon_header(AppState* state, const struct PokemonPlayer* p) {
  (void)p;
  ImGui::SetNextWindowPos({0.0f, (float)menu_bar_height});
  ImGui::SetNextWindowSize({(float)win_width, (float)POKEMON_HEADER_H});
  ImGui::SetNextWindowBgAlpha(0.92f);
  ImGui::Begin("##pkm_header", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  ImVec4 game_col = (state->gb && strstr(mmu_get_rom_title(state->gb->mmu), "RED"))
                        ? ImVec4(0.95f, 0.20f, 0.15f, 1.0f)
                        : ImVec4(0.20f, 0.40f, 0.95f, 1.0f);
  ImGui::PushStyleColor(ImGuiCol_Text, game_col);
  const char* rom_title = state->gb ? mmu_get_rom_title(state->gb->mmu) : "POKEMON";
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
  ImGui::Text("  %s", rom_title);
  ImGui::PopStyleColor();

  Uint64 elapsed_ms = SDL_GetTicks() - state->pokemon_session_start;
  unsigned secs  = (unsigned)(elapsed_ms / 1000u) % 60u;
  unsigned mins  = (unsigned)(elapsed_ms / 60000u) % 60u;
  unsigned hours = (unsigned)(elapsed_ms / 3600000u);
  char timer_buf[32];
  snprintf(timer_buf, sizeof(timer_buf), "%02u:%02u:%02u", hours, mins, secs);
  float tw = ImGui::CalcTextSize(timer_buf).x;
  ImGui::SameLine((float)win_width / 2.0f - tw / 2.0f);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY());
  ImGui::TextDisabled("%s", timer_buf);

  bool cgb = state->gb && ppu_get_cgb_mode(state->gb->ppu);
  float mode_w = ImGui::CalcTextSize(cgb ? "CGB Mode" : "DMG Mode").x;
  ImGui::SameLine((float)win_width - mode_w - 16.0f);
  ImGui::TextDisabled("%s", cgb ? "CGB Mode" : "DMG Mode");

  ImGui::End();
}

static void draw_pokemon_party(AppState* state, const struct PokemonPlayer* p) {
  float panel_y = (float)(menu_bar_height + POKEMON_HEADER_H);
  float panel_h = (float)(win_height - menu_bar_height - POKEMON_HEADER_H - POKEMON_BOTTOM_H);
  ImGui::SetNextWindowPos({0.0f, panel_y});
  ImGui::SetNextWindowSize({(float)POKEMON_LEFT_W, panel_h});
  ImGui::SetNextWindowBgAlpha(0.92f);
  ImGui::Begin("##pkm_party", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  ImGui::TextDisabled("PARTY");
  ImGui::Separator();

  float card_w = (float)(POKEMON_LEFT_W - 18) / 2.0f;
  float card_h = (panel_h - 36.0f) / 3.0f - 4.0f;

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 2; col++) {
      int slot = row * 2 + col;
      if (col > 0) ImGui::SameLine();

      const struct PokemonMon* m   = (slot < p->party_count) ? &p->party[slot] : nullptr;
      bool  is_active  = (p->in_battle && slot == (int)p->active_slot);
      bool  is_focused = (slot == state->pokemon_focused_slot);

      ImVec4 border_col = m ? status_color(m->status, m->hp)
                            : ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
      if (is_active) border_col = ImVec4(1.0f, 0.85f, 0.0f, 1.0f);
      if (is_focused && !is_active) border_col.w = 0.6f;

      ImGui::PushID(slot);
      ImGui::PushStyleColor(ImGuiCol_ChildBg,
                             m ? ImVec4(0.18f, 0.18f, 0.22f, 0.95f)
                               : ImVec4(0.12f, 0.12f, 0.15f, 0.70f));
      ImGui::PushStyleColor(ImGuiCol_Border, border_col);
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, is_focused ? 2.0f : 1.0f);

      bool clicked = false;
      if (ImGui::BeginChild("##card", {card_w, card_h}, ImGuiChildFlags_Borders)) {
        if (m && m->species) {
          float box_sz  = card_h * 0.38f;
          ImVec4 tc     = type_color(m->type1);
          ImVec2 cpos   = ImGui::GetCursorScreenPos();
          ImGui::GetWindowDrawList()->AddRectFilled(
              cpos, {cpos.x + box_sz, cpos.y + box_sz},
              ImGui::ColorConvertFloat4ToU32(tc), 4.0f);
          uint8_t dex = pokemon_internal_to_dex[m->species];
          if (dex) {
            char dex_buf[8];
            snprintf(dex_buf, sizeof(dex_buf), "#%03d", dex);
            ImGui::GetWindowDrawList()->AddText(
                {cpos.x + 2, cpos.y + 2},
                IM_COL32(255, 255, 255, 200), dex_buf);
          }
          ImGui::Dummy({box_sz, box_sz});

          const char* name = m->nickname[0] ? m->nickname
                                            : species_name(m->species);
          ImGui::Text("%s", name);
          ImGui::SameLine();
          ImGui::TextDisabled("L%d", m->level);

          float frac = m->max_hp ? (float)m->hp / (float)m->max_hp : 0.0f;
          draw_hp_bar(frac, card_w - 8.0f);
          ImGui::TextDisabled("%d/%d", m->hp, m->max_hp);
        } else {
          ImGui::TextDisabled("---");
        }
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
          clicked = true;
      }
      ImGui::EndChild();
      ImGui::PopStyleVar();
      ImGui::PopStyleColor(2);
      ImGui::PopID();

      if (clicked && m && m->species)
        state->pokemon_focused_slot = slot;
    }
  }

  ImGui::End();
}

static void draw_pokemon_right_overworld(const struct PokemonPlayer* p) {
  float panel_y = (float)(menu_bar_height + POKEMON_HEADER_H);
  float panel_h = (float)(win_height - menu_bar_height - POKEMON_HEADER_H - POKEMON_BOTTOM_H);
  ImGui::SetNextWindowPos({(float)(win_width - POKEMON_RIGHT_W), panel_y});
  ImGui::SetNextWindowSize({(float)POKEMON_RIGHT_W, panel_h});
  ImGui::SetNextWindowBgAlpha(0.92f);
  ImGui::Begin("##pkm_right", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  ImGui::TextDisabled("TRAINER");
  ImGui::Separator();

  ImGui::Text("%s", p->trainer_name[0] ? p->trainer_name : "???");
  ImGui::Text("$%u", p->money);

  ImGui::Spacing();
  ImGui::TextDisabled("BADGES");
  static const char* badge_names[8] = {
    "Boulder","Cascade","Thunder","Rainbow",
    "Soul","Marsh","Volcano","Earth"
  };
  for (int i = 0; i < 8; i++) {
    bool have = (p->badges >> i) & 1;
    if (i % 4 != 0) ImGui::SameLine();
    if (have) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.1f, 1.0f));
      ImGui::Text("[%s]", badge_names[i]);
      ImGui::PopStyleColor();
    } else {
      ImGui::TextDisabled("[%s]", badge_names[i]);
    }
  }

  ImGui::Spacing();
  ImGui::TextDisabled("LOCATION");
  const char* loc = pokemon_map_names[p->map_id];
  ImGui::Text("%s", loc ? loc : "???");

  ImGui::Spacing();
  ImGui::TextDisabled("POKEDEX");
  ImGui::Text("Seen:  %d", p->dex_seen);
  ImGui::Text("Owned: %d", p->dex_owned);

  ImGui::End();
}

static void draw_pokemon_right_battle(const struct PokemonPlayer* p) {
  float panel_y = (float)(menu_bar_height + POKEMON_HEADER_H);
  float panel_h = (float)(win_height - menu_bar_height - POKEMON_HEADER_H - POKEMON_BOTTOM_H);
  ImGui::SetNextWindowPos({(float)(win_width - POKEMON_RIGHT_W), panel_y});
  ImGui::SetNextWindowSize({(float)POKEMON_RIGHT_W, panel_h});
  ImGui::SetNextWindowBgAlpha(0.92f);
  ImGui::Begin("##pkm_right", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  if (p->active_slot >= p->party_count) {
    ImGui::TextDisabled("No active pokemon");
    ImGui::End();
    return;
  }
  const struct PokemonMon* m = &p->party[p->active_slot];

  ImGui::TextDisabled("ACTIVE");
  ImGui::Separator();

  const char* name = m->nickname[0] ? m->nickname : species_name(m->species);
  ImGui::Text("%s", name);
  ImGui::SameLine();
  ImGui::TextDisabled("L%d", m->level);

  draw_type_badge(m->type1);
  if (m->type2 != m->type1) { ImGui::SameLine(); draw_type_badge(m->type2); }

  ImGui::Spacing();
  float frac = m->max_hp ? (float)m->hp / (float)m->max_hp : 0.0f;
  draw_hp_bar(frac, (float)POKEMON_RIGHT_W - 20.0f);
  ImGui::Text("HP %d / %d", m->hp, m->max_hp);

  ImGui::Spacing();
  ImGui::TextDisabled("STATS");
  ImGui::Text("ATK %d  DEF %d", m->atk, m->def);
  ImGui::Text("SPD %d  SPC %d", m->spd, m->spc);

  ImGui::Spacing();
  ImGui::TextDisabled("MOVES");
  for (int i = 0; i < 4; i++) {
    if (!m->move_ids[i]) break;
    const char* mn = (m->move_ids[i] < 166 && pokemon_move_names[m->move_ids[i]])
                       ? pokemon_move_names[m->move_ids[i]] : "???";
    ImGui::Text("%-14s %d PP", mn, m->move_pp[i]);
  }

  ImGui::End();
}

static void draw_pokemon_bottom_overworld(const AppState* state, const struct PokemonPlayer* p) {
  ImGui::SetNextWindowPos({0.0f, (float)(win_height - POKEMON_BOTTOM_H)});
  ImGui::SetNextWindowSize({(float)win_width, (float)POKEMON_BOTTOM_H});
  ImGui::SetNextWindowBgAlpha(0.90f);
  ImGui::Begin("##pkm_bottom", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  int slot = state->pokemon_focused_slot;
  if (slot >= p->party_count) { ImGui::End(); return; }
  const struct PokemonMon* m = &p->party[slot];
  if (!m->species) { ImGui::End(); return; }

  const char* name = m->nickname[0] ? m->nickname : species_name(m->species);
  ImGui::TextDisabled("%s's MOVES", name);
  ImGui::SameLine();

  float col_w = ((float)win_width - 8.0f) / 4.0f;
  for (int i = 0; i < 4; i++) {
    if (i > 0) ImGui::SameLine();
    ImGui::BeginGroup();
    if (m->move_ids[i]) {
      const char* mn = (m->move_ids[i] < 166 && pokemon_move_names[m->move_ids[i]])
                         ? pokemon_move_names[m->move_ids[i]] : "???";
      ImGui::Text("%s", mn);
      ImGui::TextDisabled("PP %d", m->move_pp[i]);
    } else {
      ImGui::TextDisabled("---");
    }
    ImGui::Dummy({col_w, 0.0f});
    ImGui::EndGroup();
  }

  ImGui::End();
}

static void draw_pokemon_bottom_battle(const struct PokemonPlayer* p) {
  ImGui::SetNextWindowPos({0.0f, (float)(win_height - POKEMON_BOTTOM_H)});
  ImGui::SetNextWindowSize({(float)win_width, (float)POKEMON_BOTTOM_H});
  ImGui::SetNextWindowBgAlpha(0.90f);
  ImGui::Begin("##pkm_bottom", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoBringToFrontOnFocus |
               ImGuiWindowFlags_NoFocusOnAppearing);

  ImGui::TextDisabled("ENEMY");
  ImGui::SameLine();

  ImGui::Text("%s", species_name(p->enemy_species));
  ImGui::SameLine();
  ImGui::TextDisabled("L%d", p->enemy_level);
  ImGui::SameLine();
  draw_type_badge(p->enemy_type1);
  if (p->enemy_type2 != p->enemy_type1) {
    ImGui::SameLine(); draw_type_badge(p->enemy_type2);
  }

  ImGui::End();
}

void draw_pokemon_overlay(AppState* state) {
  if (!state->gb) return;
  struct PokemonPlayer player;
  pokemon_read_player(state->gb->mmu, &player);

  draw_pokemon_header(state, &player);
  draw_pokemon_party(state, &player);
  if (player.in_battle) {
    draw_pokemon_right_battle(&player);
    draw_pokemon_bottom_battle(&player);
  } else {
    draw_pokemon_right_overworld(&player);
    draw_pokemon_bottom_overworld(state, &player);
  }
}
