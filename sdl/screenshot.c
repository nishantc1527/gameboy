#include <SDL3/SDL.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gbemu/core.h"
#include "gbemu/ppu.h"
#include "stb_image_write.h"

void take_screenshot(struct Ppu* ppu) {
  static int counter = 0;
  char path[64];
  SDL_snprintf(path, sizeof(path), "screenshot_%04d.png", ++counter);

  uint8_t buf[SCRN_HEIGHT][SCRN_WIDTH][3];
  if (ppu->cgb_mode) {
    for (int y = 0; y < SCRN_HEIGHT; y++)
      for (int x = 0; x < SCRN_WIDTH; x++) {
        uint16_t rgb555 = ppu->cgb_dsp[y][x];
        uint8_t r5 = rgb555 & 0x1F;
        uint8_t g5 = (rgb555 >> 5) & 0x1F;
        uint8_t b5 = (rgb555 >> 10) & 0x1F;
        buf[y][x][0] = (r5 << 3) | (r5 >> 2);
        buf[y][x][1] = (g5 << 3) | (g5 >> 2);
        buf[y][x][2] = (b5 << 3) | (b5 >> 2);
      }
  } else {
    for (int y = 0; y < SCRN_HEIGHT; y++)
      for (int x = 0; x < SCRN_WIDTH; x++) {
        uint32_t c = g_settings.dmg_palette[ppu->dsp[y][x]];
        buf[y][x][0] = (uint8_t)(c >> 16);
        buf[y][x][1] = (uint8_t)(c >> 8);
        buf[y][x][2] = (uint8_t)(c);
      }
  }

  if (!stbi_write_png(path, SCRN_WIDTH, SCRN_HEIGHT, 3, buf, SCRN_WIDTH * 3))
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to write screenshot: %s\n",
                 path);
  else
    SDL_Log("Screenshot saved: %s\n", path);
}
