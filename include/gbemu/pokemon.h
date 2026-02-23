#pragma once

#include <stdint.h>

#include "gbemu/mmu.h"

extern uint8_t pokemon_enabled;

int p_check_pokemon(const char* rom_title);
void p_init_data(Mmu* mmu);

/* Automatically set the correct checksum value after
         hacking the save file                              */
void p_set_checksum(Mmu* mmu);

void p_get_name(Mmu* mmu, char name[]);
