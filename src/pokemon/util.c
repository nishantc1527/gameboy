#include <stdint.h>
#include <string.h>

#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "pokemon_private.h"

void p_string_convert(char dst[], size_t dst_size, uint16_t n,
                      const uint8_t encoded[]) {
  for (int i = 0; i < n; i++) {
    if (encoded[i] == 0x50) break;
    if (!p_table[encoded[i]]) continue;
    strncat(dst, p_table[encoded[i]], dst_size - strlen(dst) - 1);
  }
}

void p_set_checksum(struct Mmu* mmu) {
  uint8_t sum = 255;
  for (uint16_t loc = 0x2598; loc <= 0x3522; loc++) {
    sum -= mmu_read_extern_ram(mmu, loc);
  }
  mmu_write_extern_ram(mmu, 0x3523, sum);
}

int pokemon_check(const char* rom_title) {
  return !strcmp(rom_title, "POKEMON RED") || !strcmp(rom_title, "POKEMON BLUE");
}

void pokemon_init(struct Mmu* mmu) {
  p_set_checksum(mmu);
  p_init_table();
}
