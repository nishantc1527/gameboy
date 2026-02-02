#include <stdint.h>

#include "gbemu/mmu.h"

uint8_t no_mbc_read_rom(uint16_t loc) { return rom[loc]; }

uint8_t no_mbc_read_ram(uint16_t loc __attribute__((unused))) { return 0xFF; }

void no_mbc_write_rom(uint16_t loc __attribute__((unused)),
                      uint8_t val __attribute__((unused))) {}

void no_mbc_write_ram(uint16_t loc __attribute__((unused)),
                      uint8_t val __attribute__((unused))) {}
