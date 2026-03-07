#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <string.h>

#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "internal.h"

uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];

void render(struct Ppu* ppu) {
  if (ppu->cgb_mode) {
    for (int i = 0; i < SCRN_HEIGHT; i++)
      for (int j = 0; j < SCRN_WIDTH; j++) {
        uint16_t rgb555 = ppu->cgb_dsp[i][j];
        uint8_t r5 = rgb555 & 0x1F;
        uint8_t g5 = (rgb555 >> 5) & 0x1F;
        uint8_t b5 = (rgb555 >> 10) & 0x1F;
        uint32_t r = (r5 << 3) | (r5 >> 2);
        uint32_t g = (g5 << 3) | (g5 >> 2);
        uint32_t b = (b5 << 3) | (b5 >> 2);
        buf[i][j] = (r << 24) | (g << 16) | (b << 8) | SDL_ALPHA_OPAQUE;
      }
  } else {
    uint32_t pal[4];
    for (int i = 0; i < 4; i++)
      pal[i] = (g_settings.dmg_palette[i] << 8) | (uint32_t)SDL_ALPHA_OPAQUE;
    for (int i = 0; i < SCRN_HEIGHT; i++)
      for (int j = 0; j < SCRN_WIDTH; j++) buf[i][j] = pal[ppu->dsp[i][j]];
  }
  SDL_UpdateTexture(txt, NULL, buf, SCRN_WIDTH * sizeof(uint32_t));
  SDL_RenderTexture(rnd, txt, NULL, NULL);
}

static void draw_ui_idle(AppState* state) {
  int pw = 300;
  int row_h = 30;
  int recent = g_settings.recent_rom_count;
  int ph = 60 + (recent > 0 ? 25 + recent * (row_h + 2) : 0);
  int px = (win_width - pw) / 2;
  int py = (win_height - ph) / 2;

  if (nk_begin(ctx, "gbemu",
               nk_rect((float)px, (float)py, (float)pw, (float)ph),
               NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, row_h, 1);
    if (nk_button_label(ctx, "Open ROM...")) open_rom_dialog(state);

    if (recent > 0) {
      nk_layout_row_dynamic(ctx, 16, 1);
      nk_label(ctx, "Recent:", NK_TEXT_LEFT);
      for (int i = 0; i < recent; i++) {
        const char* path = g_settings.recent_roms[i];
        const char* name = strrchr(path, '/');
        name = name ? name + 1 : path;
        nk_layout_row_dynamic(ctx, row_h, 1);
        if (nk_button_label(ctx, name))
          SDL_snprintf(state->pending_rom, sizeof(state->pending_rom), "%s",
                       path);
      }
    }
  }
  nk_end(ctx);
}

static void draw_ui_playing(AppState* state) { (void)state; }

void draw_ui(AppState* state) {
  SDL_SetRenderLogicalPresentation(rnd, win_width, win_height,
                                   SDL_LOGICAL_PRESENTATION_DISABLED);

  if (!state->gb)
    draw_ui_idle(state);
  else
    draw_ui_playing(state);

  nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);

  SDL_SetRenderLogicalPresentation(rnd, SCRN_WIDTH, SCRN_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
}
