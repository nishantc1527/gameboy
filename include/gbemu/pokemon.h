#pragma once

#include <stddef.h>
#include <stdint.h>

struct Mmu;

extern uint8_t pokemon_enabled;

int p_check_pokemon(const char* rom_title);
void p_init_data(struct Mmu* mmu);

/* Automatically set the correct checksum value after
         hacking the save file                              */
void p_set_checksum(struct Mmu* mmu);

void p_get_name(struct Mmu* mmu, char name[], size_t name_size);
