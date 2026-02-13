#include <SDL3/SDL_log.h>
#include <string.h>

#include "gbemu/mmu.h"
#include "internal.h"

void p_string_convert(char dst[], uint16_t n, uint8_t encoded[]) {
  for (int i = 0; i < n; i++) {
    if (encoded[i] == 0x50) break;
    strcat(dst, p_table[encoded[i]]);
  }
}

void p_set_checksum(void) {
  uint8_t sum = 255;
  for (uint16_t loc = 0x2598; loc <= 0x3522; loc++)
    sum -= mmu_r_ram_raw(mmu, loc);
  mmu_w_ram_raw(mmu, 0x3523, sum);
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Set the new checksum: $%02X\n",
  // sum);
}

int p_check_pokemon(const char* rom_title) {
  if (!strcmp(rom_title, "POKEMON RED") || !strcmp(rom_title, "POKEMON BLUE"))
    return 1;
  return 0;
}
