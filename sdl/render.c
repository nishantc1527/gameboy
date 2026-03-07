#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
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

void draw_ui(void) {
  if (nk_begin(ctx, "Show", nk_rect(0, 0, SCRN_WIDTH * g_settings.scale, 35),
               NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 35, 1);
    if (nk_button_label(ctx, "File")) {
    }
  }
  nk_end(ctx);
  nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);
}
