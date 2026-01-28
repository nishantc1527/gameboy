#pragma once

#include <stdint.h>

extern uint8_t pokemon_enabled;

int p_check_pokemon(const char* rom_title);
void p_init_data(void);

/* Automatically set the correct checksum value after
         hacking the save file                              */
void p_set_checksum(void);

void p_get_name(char name[]);
