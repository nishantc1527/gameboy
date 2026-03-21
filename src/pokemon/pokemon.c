#include "gbemu/pokemon.h"

#include <stdint.h>

#include "gbemu/mmu.h"
#include "internal.h"

uint8_t pokemon_enabled;
char* p_table[0x100];

void p_init_data(struct Mmu* mmu) {
  p_set_checksum(mmu);
  p_init_table();
}

void p_get_name(struct Mmu* mmu, char name[]) {
  const uint16_t n = 0xB;
  const uint16_t src = 0x2598;
  uint8_t enc[n];
  for (uint16_t i = src; i < src + n; i++) enc[i - src] = mmu_r_ram_raw(mmu, i);
  p_string_convert(name, n, enc);
}

void p_init_table() {
  static char p_buf[0x100][2];
  p_table[0x7F] = "␠";
  for (int i = 0x80; i <= 0x99; i++) {
    p_buf[i][0] = (char)(i - 0x80 + 'A');
    p_buf[i][1] = '\0';
    p_table[i] = p_buf[i];
  }
  p_table[0x9A] = "(";
  p_table[0x9B] = ")";
  p_table[0x9C] = ":";
  p_table[0x9D] = ";";
  p_table[0x9E] = "[";
  p_table[0x9F] = "]";
  for (int i = 0xA0; i <= 0xB9; i++) {
    p_buf[i][0] = (char)(i - 0xA0 + 'a');
    p_buf[i][1] = '\0';
    p_table[i] = p_buf[i];
  }
  p_table[0xBA] = "é";
  p_table[0xBB] = "'d";
  p_table[0xBC] = "'l";
  p_table[0xBD] = "'s";
  p_table[0xBE] = "'t";
  p_table[0xBF] = "'v";
  p_table[0xE0] = "'";
  p_table[0xE1] = "PK";
  p_table[0xE2] = "MN";
  p_table[0xE3] = "-";
  p_table[0xE4] = "'r";
  p_table[0xE5] = "'m";
  p_table[0xE6] = "?";
  p_table[0xE7] = "!";
  p_table[0xE8] = ".";
  p_table[0xE9] = "ァ";
  p_table[0xEA] = "ゥ";
  p_table[0xEB] = "ェ";
  p_table[0xEC] = "▷";
  p_table[0xEE] = "▼";
  p_table[0xEF] = "♂";
  p_table[0xF1] = "×";
  p_table[0xF2] = ".";
  p_table[0xF3] = "/";
  p_table[0xF4] = ",";
  p_table[0xF5] = "♀";
  for (int i = 0xF6; i <= 0xFF; i++) {
    p_buf[i][0] = (char)(i - 0xF6 + '0');
    p_buf[i][1] = '\0';
    p_table[i] = p_buf[i];
  }
}
