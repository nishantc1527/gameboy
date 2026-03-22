#pragma once

#include <stdint.h>

extern char* p_table[0x100];

void p_init_table();

// From pokemon string format to a string
void p_string_convert(char dst[], uint16_t n, uint8_t encoded[]);
