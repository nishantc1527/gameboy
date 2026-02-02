#include <SDL3/SDL_log.h>
#include <stdint.h>

#include "gbemu/mmu.h"
#include "internal.h"

uint8_t mbc1_read_rom(uint16_t loc) {
  if (loc < 0x4000)
    return rom[loc];
  else
    return rom[loc + 0x4000 * (rom_bank - 1)];
}

uint8_t mbc1_read_ram(uint16_t loc) {
  if (ram_enable) {
    switch (ram_size) {
      case 0x00:
        return 0xFF;
      case 0x02:
        return extern_ram[loc - 0xA000];  //  8 kilouint8_ts - 1 bank
      case 0x03:
        uint16_t _loc = loc - 0xA000 + 0x2000 * ram_bank;
        return extern_ram[_loc];  // 32 kilouint8_ts - 4 banks
    }
  }
  return 0xFF;
}

void mbc1_write_rom(uint16_t loc, uint8_t val) {
  if (loc < 0x2000) {
    if ((val & 0xF) == 0xA)
      ram_enable = 1;
    else
      ram_enable = 0;
  } else if (loc < 0x4000) {
    rom_bank = val & 0b11111;
    if (rom_bank == 0) rom_bank = 1;
    rom_bank &= (1 << (rom_size + 1)) - 1;
  } else if (loc < 0x6000) {
    if (ram_bank == 0x03) {
      val &= 0b11;
      ram_bank = val;
    }
  } else if (loc < 0x8000) {
    mbc1_1mb_mode = val & 1;
  }
}

void mbc1_write_ram(uint16_t loc, uint8_t val) {
  if (ram_enable) extern_ram[loc - 0xA000] = val;
}
