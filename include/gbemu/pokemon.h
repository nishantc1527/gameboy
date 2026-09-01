#pragma once

#include <stddef.h>
#include <stdint.h>

struct Mmu;

struct PokemonMon {
  uint8_t  species;
  char     nickname[12];
  uint8_t  level;
  uint16_t hp, max_hp;
  uint8_t  status;
  uint8_t  type1, type2;
  uint8_t  move_ids[4];
  uint8_t  move_pp[4];
  uint16_t atk, def, spd, spc;
  uint32_t exp;
};

struct PokemonPlayer {
  char              trainer_name[12];
  uint8_t           party_count;
  struct PokemonMon party[6];
  uint8_t           badges;
  uint32_t          money;
  uint16_t          dex_seen, dex_owned;
  uint8_t           map_id;
  uint8_t           in_battle;
  uint8_t           active_slot;
  uint8_t           enemy_species, enemy_level, enemy_type1, enemy_type2;
};

int  pokemon_check(const char* rom_title);
void pokemon_init(struct Mmu* mmu);
void pokemon_read_player(struct Mmu* mmu, struct PokemonPlayer* out);

extern const char* const pokemon_species_names[256];
extern const char* const pokemon_move_names[166];
extern const char* const pokemon_type_names[27];
extern const char* const pokemon_map_names[256];
extern const uint8_t     pokemon_internal_to_dex[256];
