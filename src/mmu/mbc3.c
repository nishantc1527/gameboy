#include <stdint.h>
#include <stdio.h>

#include "gbemu/mmu.h"
#include "internal.h"

uint8_t mbc3_read_rom(uint16_t loc) {
  if (loc < 0x4000)
    return rom[loc];
  else
    return rom[loc + 0x4000 * (rom_bank - 1)];
}

uint8_t mbc3_read_ram(uint16_t loc) {
  if (ram_enable) {
    if (ram_bank <= 0x03) {
      switch (ram_size) {
        case 0x00:
          return 0xFF;
        case 0x02:
          return extern_ram[loc - 0xA000];
        case 0x03: {
          uint16_t _loc = loc - 0xA000 + 0x2000 * ram_bank;
          return extern_ram[_loc];
        }
      }
    } else if (loc >= 0x08 && loc <= 0x0C) {
      // TODO
    }
  }
  return 0xFF;
}

void mbc3_write_rom(uint16_t loc, uint8_t val) {
  if (loc < 0x2000) {
    if ((val & 0xF) == 0xA)
      ram_enable = 1;
    else
      ram_enable = 0;
  } else if (loc < 0x4000) {
    uint8_t old = rom_bank;
    rom_bank = val & 0b1111111;
    if (rom_bank == 0) rom_bank++;
    if (old != rom_bank) {
      printf("new bank: %d\n", rom_bank);
    }
  } else if (loc < 0x6000) {
    ram_bank = val;
  } else if (loc < 0x8000) {
    // TODO
  }
}

void mbc3_write_ram(uint16_t loc, uint8_t val) {
  if (ram_enable) {
    if (ram_bank <= 0x03) {
      switch (ram_size) {
        case 0x00:
          return;
        case 0x02:
          extern_ram[loc - 0xA000] = val;
          return;
        case 0x03: {
          uint16_t _loc = loc - 0xA000 + 0x2000 * ram_bank;
          extern_ram[_loc] = val;
        }
      }
    } else if (ram_bank >= 0x08 && ram_bank <= 0x0C) {
      // TODO
    }
  }
}
