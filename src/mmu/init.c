#include <SDL3/SDL_log.h>
#include <stdint.h>
#include <stdio.h>

#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "internal.h"

char rom_title[20];
uint8_t cart_type;
uint8_t rom_size, rom_bank, upper_rom_bank;
uint8_t ram_size, ram_bank;
int ram_enable;
int mbc1_1mb_mode;
int mbc3_rtc_reg;

int get_rom_info(void) {
  for (uint16_t i = 0x0134; i <= 0x0142 && mem[i]; i++)
    rom_title[i - 0x0134] = (char)mem[i];
  pokemon_enabled = p_check_pokemon(rom_title);
  // if (pokemon_enabled) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"POKEMON
  // DETECTED\n");
  cart_type = mem[0x0147];
  rom_size = mem[0x0148];
  ram_size = mem[0x0149];
  switch (cart_type) {
    case 0x00:
    case 0x01:
    case 0x03:
    case 0x13:
      break;
    default:
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNIMPLEMENTED MAPPER $%02X\n",
                   cart_type);
      return 1;
  }
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "USING MAPPER: $%02X\n",
  // cart_type);
  switch (rom_size) {
    case 0x00:
    case 0x01:
    case 0x03:
    case 0x04:
    case 0x05:
      break;
    default:
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNIMPLEMENTED ROM SIZE $%02X\n",
                   rom_size);
      return 1;
  }
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "USING ROM SIZE: $%02X\n",
  // rom_size);
  switch (ram_size) {
    case 0x00:
    case 0x02:
    case 0x03:
      break;
    default:
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UNIMPLEMENTED RAM SIZE $%02X\n",
                   ram_size);
      return 1;
  }
  // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "USING RAM SIZE: $%02X\n",
  // ram_size);
  rom_bank = 1;
  ram_enable = 0;
  if (cart_type == 0x01 || cart_type == 0x03) {
    mbc1_1mb_mode = 0;
    upper_rom_bank = 0;
    if (rom_size > 0x06) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ROM SIZE NOT AVAILABLE\n");
      return 1;
    }
    if (ram_size > 0x03) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "RAM SIZE NOT AVAILABLE\n");
      return 1;
    }
  }
  if (cart_type == 0x13) {
    if (rom_size > 0x06) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ROM SIZE NOT AVAILABLE\n");
      return 1;
    }
    if (ram_size > 0x03) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "RAM SIZE NOT AVAILABLE\n");
      return 1;
    }
  }
  return 0;
}
