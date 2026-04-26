#include <SDL3/SDL_render.h>

#include "gbemu/core.h"
#include "gbemu/ppu.h"
#include "gbemu/sdl.h"
#include "gbemu/settings.h"
#include "sdl_private.h"

#define RGB555_MASK 0x1F
#define RGB555_RED_SHIFT 0
#define RGB555_GREEN_SHIFT 5
#define RGB555_BLUE_SHIFT 10
#define RGB555_EXPAND_SHIFT 3
#define RGB555_FILL_SHIFT 2
#define RGBA_R_SHIFT 24
#define RGBA_G_SHIFT 16
#define RGBA_B_SHIFT 8

uint32_t buf[SCRN_HEIGHT][SCRN_WIDTH];

void render(struct Ppu* ppu) {
  if (ppu_get_cgb_mode(ppu)) {
    const uint16_t* cgb_fb = ppu_get_cgb_framebuffer(ppu);
    for (int i = 0; i < SCRN_HEIGHT; i++)
      for (int j = 0; j < SCRN_WIDTH; j++) {
        uint16_t rgb555 = cgb_fb[i * SCRN_WIDTH + j];
        uint8_t r5 = (rgb555 >> RGB555_RED_SHIFT) & RGB555_MASK;
        uint8_t g5 = (rgb555 >> RGB555_GREEN_SHIFT) & RGB555_MASK;
        uint8_t b5 = (rgb555 >> RGB555_BLUE_SHIFT) & RGB555_MASK;
        uint32_t r = (uint32_t)(r5 << RGB555_EXPAND_SHIFT) |
                     (uint32_t)(r5 >> RGB555_FILL_SHIFT);
        uint32_t g = (uint32_t)(g5 << RGB555_EXPAND_SHIFT) |
                     (uint32_t)(g5 >> RGB555_FILL_SHIFT);
        uint32_t b = (uint32_t)(b5 << RGB555_EXPAND_SHIFT) |
                     (uint32_t)(b5 >> RGB555_FILL_SHIFT);
        buf[i][j] = (r << RGBA_R_SHIFT) | (g << RGBA_G_SHIFT) |
                    (b << RGBA_B_SHIFT) | SDL_ALPHA_OPAQUE;
      }
  } else {
    const uint8_t* dmg_fb = ppu_get_dmg_framebuffer(ppu);
    uint32_t pal[4];
    for (int i = 0; i < 4; i++)
      pal[i] = (g_settings.dmg_palette[i] << 8) | (uint32_t)SDL_ALPHA_OPAQUE;
    for (int i = 0; i < SCRN_HEIGHT; i++)
      for (int j = 0; j < SCRN_WIDTH; j++)
        buf[i][j] = pal[dmg_fb[i * SCRN_WIDTH + j]];
  }
  SDL_UpdateTexture(txt, NULL, buf, SCRN_WIDTH * sizeof(uint32_t));
  int avail_h = win_height - menu_bar_height;
  int scale = win_width / SCRN_WIDTH;
  if (avail_h / SCRN_HEIGHT < scale) scale = avail_h / SCRN_HEIGHT;
  if (scale < 1) scale = 1;
  SDL_FRect dest = {
      (float)((win_width - SCRN_WIDTH * scale) / 2),
      (float)(menu_bar_height + (avail_h - SCRN_HEIGHT * scale) / 2),
      (float)(SCRN_WIDTH * scale),
      (float)(SCRN_HEIGHT * scale),
  };
  SDL_RenderTexture(rnd, txt, NULL, &dest);
}
