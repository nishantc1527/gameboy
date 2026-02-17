#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "colors.h"
#include "display.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"
#include "internal.h"

uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];

static const uint32_t pal_lut[] = {
    [0] = ((uint32_t)(HEX_WHT >> 16) << 24) | ((uint32_t)((HEX_WHT >> 8) & 0xFF) << 16) |
          ((uint32_t)(HEX_WHT & 0xFF) << 8) | (uint32_t)SDL_ALPHA_OPAQUE,
    [1] = ((uint32_t)(HEX_L_GREY >> 16) << 24) |
          ((uint32_t)((HEX_L_GREY >> 8) & 0xFF) << 16) |
          ((uint32_t)(HEX_L_GREY & 0xFF) << 8) | (uint32_t)SDL_ALPHA_OPAQUE,
    [2] = ((uint32_t)(HEX_R_GREY >> 16) << 24) |
          ((uint32_t)((HEX_R_GREY >> 8) & 0xFF) << 16) |
          ((uint32_t)(HEX_R_GREY & 0xFF) << 8) | (uint32_t)SDL_ALPHA_OPAQUE,
    [3] = ((uint32_t)(HEX_BLK >> 16) << 24) | ((uint32_t)((HEX_BLK >> 8) & 0xFF) << 16) |
          ((uint32_t)(HEX_BLK & 0xFF) << 8) | (uint32_t)SDL_ALPHA_OPAQUE,
    [4] = ((uint32_t)(HEX_BLK >> 16) << 24) | ((uint32_t)((HEX_BLK >> 8) & 0xFF) << 16) |
          ((uint32_t)(HEX_BLK & 0xFF) << 8) | (uint32_t)SDL_ALPHA_OPAQUE,
};

void render(void) {
  for (int i = 0; i < SCRN_HEIGHT; i++) {
    for (int j = 0; j < SCRN_WIDTH; j++) {
      uint8_t clr = dsp[i][j];
      buf[i][j] = pal_lut[clr];
    }
  }
  SDL_UpdateTexture(txt, NULL, buf, SCRN_WIDTH * sizeof(uint32_t));
  SDL_RenderTexture(
      rnd, txt, NULL,
      &(SDL_FRect){0, 0, SCRN_WIDTH * SCALE_X, SCRN_HEIGHT * SCALE_Y});
}

void draw_ui(void) {
  if (nk_begin(ctx, "Show", nk_rect(0, 0, SCRN_WIDTH * SCALE_X, 35),
               NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 35, 1);
    if (nk_button_label(ctx, "File")) {
    }
  }
  nk_end(ctx);
  nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);
}
