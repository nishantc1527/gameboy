#include <SDL3/SDL_log.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "internal.h"

uint8_t mem[0x10000];
uint8_t brom[0x100];
uint8_t rom[0x800000];
uint8_t extern_ram[0x20000];

uint8_t r_mem(uint16_t loc) {
  if (!mem[0xFF50] && loc < 0x100) return brom[loc];
  if (loc < 0x8000) {
    switch (cart_type) {
      case 0x00:
        return no_mbc_read_rom(loc);
      case 0x01:
      case 0x03:
        return mbc1_read_rom(loc);
      case 0x13:
        return mbc3_read_rom(loc);
    }
  }
  if (loc >= 0xA000 && loc < 0xC000) {
    switch (cart_type) {
      case 0x00:
        return no_mbc_read_ram(loc);
      case 0x01:
      case 0x03:
        return mbc1_read_ram(loc);
      case 0x13:
        return mbc3_read_ram(loc);
    }
  }
  if (loc >= 0xE000 && loc <= 0xFDFF) loc -= 0x200;
  return mem[loc];
}

void w_mem(uint16_t loc, uint8_t val) {
  if (loc < 0x8000) {
    switch (cart_type) {
      case 0x00:
        no_mbc_write_rom(loc, val);
        return;
      case 0x01:
      case 0x03:
        mbc1_write_rom(loc, val);
        return;
      case 0x13:
        mbc3_write_rom(loc, val);
        return;
    }
  }
  if (loc >= 0xA000 && loc < 0xC000) {
    switch (cart_type) {
      case 0x00:
      case 0x01:
        no_mbc_write_ram(loc, val);
        return;
      case 0x13:
        mbc3_write_ram(loc, val);
        return;
    }
  }
  if (loc >= 0xE000 && loc <= 0xFDFF) loc -= 0x200;
  if (loc == 0xFF04) val = 0;
  if (loc == 0xFF07 && (val >> 2) & 1) {
    switch (val & 0b11) {
      case 0b00:
        tim_thresh = TIM_FREQ_1;
        break;
      case 0b01:
        tim_thresh = TIM_FREQ_2;
        break;
      case 0b10:
        tim_thresh = TIM_FREQ_3;
        break;
      case 0b11:
        tim_thresh = TIM_FREQ_4;
        break;
    }
    tim_thresh = CPU_FREQ / tim_thresh;
  }
  if (loc == 0xFF02 && test_category == TEST_BLARGG && val == 0x81) {
    // printf("%c", SB);
    test_out[test_out_idx++] = SB;
    test_out[test_out_idx] = '\0';
  }
  mem[loc] = val;
}

uint8_t rd8(void) { return r_mem(++PC); }

uint16_t rd16(void) {
  uint16_t addr1 = ++PC;
  uint16_t addr2 = ++PC;
  return ((uint16_t)r_mem(addr2) << 8) | (uint16_t)r_mem(addr1);
}

void push(uint16_t val) {
  uint8_t val1 = (uint8_t)(val >> 8);
  uint8_t val2 = (uint8_t)val;
  w_mem(SP - 1, val1);
  w_mem(SP - 2, val2);
  SP -= 2;
}

uint16_t pop(void) {
  uint16_t val1 = r_mem(SP);
  uint16_t val2 = r_mem(SP + 1);
  SP += 2;
  return val1 | (val2 << 8);
}

uint16_t pk(void) {
  uint16_t val = pop();
  push(val);
  return val;
}

void kp(void) { PC--; }

int save(void) {
  switch (cart_type) {
    case 0x03:
    case 0x13: {
      char file_name[30] = "";
      strcat(file_name, rom_title);
      strcat(file_name, ".sav");
      FILE* save_file = fopen(file_name, "wb");
      if (!save_file) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT SAVE TO FILE: %s\n",
                     file_name);
        return 1;
      }
      fwrite(extern_ram, 1, sizeof(extern_ram), save_file);
      fclose(save_file);
      // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SAVED GAME TO: %s\n",
      // file_name);
      return 0;
    }
  }
  return 1;
}

int load(void) {
  switch (cart_type) {
    case 0x03:
    case 0x13: {
      char file_name[30] = "";
      strcat(file_name, rom_title);
      strcat(file_name, ".sav");
      FILE* save_file = fopen(file_name, "rb");
      if (!save_file) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT LOAD SAVE FILE: %s\n",
                     file_name);
        return 1;
      }
      // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOADED SAVE FILE: %s\n",
      // file_name);
      if (!fread(extern_ram, 0x20000, 1, save_file)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "COULD NOT READ SAVE FILE\n");
        return 1;
      }
      fclose(save_file);
      return 0;
    }
  }
  return 1;
}
