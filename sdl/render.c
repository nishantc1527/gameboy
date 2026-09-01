#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <stdint.h>

#include "gbemu/core.h"
#include "gbemu/pokemon.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "sdl_private.h"

enum {
  RGB555_MASK = 0x1F,
  RGB555_RED_SHIFT = 0,
  RGB555_GREEN_SHIFT = 5,
  RGB555_BLUE_SHIFT = 10,
  RGB555_EXPAND_SHIFT = 3,
  RGB555_FILL_SHIFT = 2,
  RGBA_R_SHIFT = 24,
  RGBA_G_SHIFT = 16,
  RGBA_B_SHIFT = 8
};

uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];

void render(struct Ppu* ppu) {
  if (ppu_get_cgb_mode(ppu)) {
    const uint16_t* cgb_fb = ppu_get_cgb_framebuffer(ppu);
    for (int i = 0; i < SCRN_HEIGHT; i++) {
      for (int j = 0; j < SCRN_WIDTH; j++) {
        uint16_t rgb555 = cgb_fb[(i * SCRN_WIDTH) + j];
        uint8_t red5 =
            (uint8_t)(((unsigned)rgb555 >> RGB555_RED_SHIFT) & RGB555_MASK);
        uint8_t grn5 =
            (uint8_t)(((unsigned)rgb555 >> RGB555_GREEN_SHIFT) & RGB555_MASK);
        uint8_t blu5 =
            (uint8_t)(((unsigned)rgb555 >> RGB555_BLUE_SHIFT) & RGB555_MASK);
        uint32_t red = ((unsigned)red5 << RGB555_EXPAND_SHIFT) |
                       ((unsigned)red5 >> RGB555_FILL_SHIFT);
        uint32_t grn = ((unsigned)grn5 << RGB555_EXPAND_SHIFT) |
                       ((unsigned)grn5 >> RGB555_FILL_SHIFT);
        uint32_t blu = ((unsigned)blu5 << RGB555_EXPAND_SHIFT) |
                       ((unsigned)blu5 >> RGB555_FILL_SHIFT);
        buf[i][j] = (red << RGBA_R_SHIFT) | (grn << RGBA_G_SHIFT) |
                    (blu << RGBA_B_SHIFT) | SDL_ALPHA_OPAQUE;
      }
    }
  } else {
    const uint8_t* dmg_fb = ppu_get_dmg_framebuffer(ppu);
    uint32_t pal[4];
    for (int i = 0; i < 4; i++) {
      pal[i] = (g_settings.dmg_palette[i] << 8U) | (uint32_t)SDL_ALPHA_OPAQUE;
    }
    for (int i = 0; i < SCRN_HEIGHT; i++) {
      for (int j = 0; j < SCRN_WIDTH; j++) {
        buf[i][j] = pal[dmg_fb[(i * SCRN_WIDTH) + j]];
      }
    }
  }
  SDL_UpdateTexture(txt, NULL, buf, SCRN_WIDTH * sizeof(uint32_t));

  SDL_FRect dest;
  if (pokemon_enabled) {
    int game_w = win_width - POKEMON_LEFT_W - POKEMON_RIGHT_W;
    int game_h = win_height - menu_bar_height - POKEMON_HEADER_H - POKEMON_BOTTOM_H;
    int scale  = game_w / SCRN_WIDTH;
    if (game_h / SCRN_HEIGHT < scale) scale = game_h / SCRN_HEIGHT;
    if (scale < 1) scale = 1;
    dest = (SDL_FRect){
        (float)POKEMON_LEFT_W + (float)(game_w - SCRN_WIDTH * scale) / 2.0F,
        (float)(menu_bar_height + POKEMON_HEADER_H) +
            (float)(game_h - SCRN_HEIGHT * scale) / 2.0F,
        (float)(SCRN_WIDTH * scale),
        (float)(SCRN_HEIGHT * scale),
    };
  } else {
    int avail_h = win_height - menu_bar_height;
    int scale   = win_width / SCRN_WIDTH;
    if (avail_h / SCRN_HEIGHT < scale) scale = avail_h / SCRN_HEIGHT;
    if (scale < 1) scale = 1;
    dest = (SDL_FRect){
        (float)(win_width - SCRN_WIDTH * scale) / 2.0F,
        (float)menu_bar_height +
            (float)(avail_h - SCRN_HEIGHT * scale) / 2.0F,
        (float)(SCRN_WIDTH * scale),
        (float)(SCRN_HEIGHT * scale),
    };
  }
  SDL_RenderTexture(rnd, txt, NULL, &dest);
}
