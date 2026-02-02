#pragma once

#include <stdint.h>

extern uint8_t rom_bank, upper_rom_bank;
extern uint8_t ram_bank;
extern int ram_enable;
extern int mbc1_1mb_mode;
extern int mbc3_rtc_reg;

uint8_t mbc1_read_rom(uint16_t loc);
uint8_t mbc1_read_ram(uint16_t loc);

void mbc1_write_rom(uint16_t loc, uint8_t val);
void mbc1_write_ram(uint16_t loc, uint8_t val);

uint8_t mbc3_read_rom(uint16_t loc);
uint8_t mbc3_read_ram(uint16_t loc);

void mbc3_write_rom(uint16_t loc, uint8_t val);
void mbc3_write_ram(uint16_t loc, uint8_t val);

uint8_t no_mbc_read_rom(uint16_t loc);
uint8_t no_mbc_read_ram(uint16_t loc);

void no_mbc_write_rom(uint16_t loc, uint8_t val);
void no_mbc_write_ram(uint16_t loc, uint8_t val);
