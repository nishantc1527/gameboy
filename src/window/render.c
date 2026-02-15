#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "colors.h"
#include "display.h"
#include "gbemu/ppu.h"
#include "gbemu/window.h"
#include "internal.h"

uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];

void render(void) {
  for (int i = 0; i < SCRN_HEIGHT; i++) {
    for (int j = 0; j < SCRN_WIDTH; j++) {
      int clr = dsp[i][j];
      if (clr == CLR_WHT) clr = HEX_WHT;
      if (clr == CLR_L_GRY) clr = HEX_L_GREY;
      if (clr == CLR_D_GRY) clr = HEX_R_GREY;
      if (clr == CLR_BLK) clr = HEX_BLK;
      uint8_t r = (uint8_t)(clr >> 8 * 2);
      uint8_t g = (uint8_t)(clr >> 8 * 1);
      uint8_t b = (uint8_t)(clr >> 8 * 0);
      uint32_t col = ((uint32_t)r << 24) | ((uint32_t)g << 16) |
                     ((uint32_t)b << 8) | ((uint32_t)SDL_ALPHA_OPAQUE << 0);
      buf[i][j] = col;
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
