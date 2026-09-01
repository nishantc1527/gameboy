#pragma once

#include <stddef.h>
#include <stdint.h>

struct Mmu;

extern char* p_table[0x100];

void     p_init_table(void);
void     p_string_convert(char dst[], size_t dst_size, uint16_t n, const uint8_t encoded[]);
void     p_set_checksum(struct Mmu* mmu);
uint32_t p_decode_bcd(struct Mmu* mmu, uint16_t addr, int n);
int      p_dex_count(struct Mmu* mmu, uint16_t addr);
