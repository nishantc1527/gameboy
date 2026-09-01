#include <string.h>

#include "gbemu/mmu.h"
#include "gbemu/pokemon.h"
#include "pokemon_private.h"

enum {
  OFF_SPECIES   = 0x00,
  OFF_HP_HI     = 0x01,
  OFF_HP_LO     = 0x02,
  OFF_STATUS    = 0x04,
  OFF_TYPE1     = 0x05,
  OFF_TYPE2     = 0x06,
  OFF_MOVE0     = 0x08,
  OFF_MOVE1     = 0x09,
  OFF_MOVE2     = 0x0A,
  OFF_MOVE3     = 0x0B,
  OFF_EXP0      = 0x0E,
  OFF_EXP1      = 0x0F,
  OFF_EXP2      = 0x10,
  OFF_PP0       = 0x1D,
  OFF_PP1       = 0x1E,
  OFF_PP2       = 0x1F,
  OFF_PP3       = 0x20,
  OFF_LEVEL     = 0x21,
  OFF_MAXHP_HI  = 0x22,
  OFF_MAXHP_LO  = 0x23,
  OFF_ATK_HI    = 0x24,
  OFF_ATK_LO    = 0x25,
  OFF_DEF_HI    = 0x26,
  OFF_DEF_LO    = 0x27,
  OFF_SPD_HI    = 0x28,
  OFF_SPD_LO    = 0x29,
  OFF_SPC_HI    = 0x2A,
  OFF_SPC_LO    = 0x2B,
  PARTY_STRIDE  = 44,
};

static uint32_t decode_bcd(struct Mmu* mmu, uint16_t addr, int n) {
  uint32_t result = 0;
  for (int i = 0; i < n; i++) {
    uint8_t byte = mmu_read_wram(mmu, addr + (uint16_t)i);
    result = result * 100u + (uint32_t)(((byte >> 4) & 0xF) * 10 + (byte & 0xF));
  }
  return result;
}

static int dex_count(struct Mmu* mmu, uint16_t addr) {
  int count = 0;
  for (int i = 0; i < 19; i++) {
    uint8_t byte = mmu_read_wram(mmu, addr + (uint16_t)i);
    while (byte) {
      count += (byte & 1);
      byte >>= 1;
    }
  }
  return count;
}

void pokemon_read_player(struct Mmu* mmu, struct PokemonPlayer* out) {
  memset(out, 0, sizeof(*out));

  uint8_t enc[11];
  for (int i = 0; i < 11; i++)
    enc[i] = mmu_read_wram(mmu, 0xD158u + (uint16_t)i);
  p_string_convert(out->trainer_name, sizeof(out->trainer_name), 11, enc);

  out->party_count = mmu_read_wram(mmu, 0xD163);
  if (out->party_count > 6) out->party_count = 6;

  for (int i = 0; i < out->party_count; i++) {
    uint16_t base = 0xD16Bu + (uint16_t)(i * PARTY_STRIDE);
    struct PokemonMon* m = &out->party[i];

    m->species = mmu_read_wram(mmu, base + OFF_SPECIES);
    m->hp = ((uint16_t)mmu_read_wram(mmu, base + OFF_HP_HI) << 8) |
             mmu_read_wram(mmu, base + OFF_HP_LO);
    m->status = mmu_read_wram(mmu, base + OFF_STATUS);
    m->type1  = mmu_read_wram(mmu, base + OFF_TYPE1);
    m->type2  = mmu_read_wram(mmu, base + OFF_TYPE2);

    m->move_ids[0] = mmu_read_wram(mmu, base + OFF_MOVE0);
    m->move_ids[1] = mmu_read_wram(mmu, base + OFF_MOVE1);
    m->move_ids[2] = mmu_read_wram(mmu, base + OFF_MOVE2);
    m->move_ids[3] = mmu_read_wram(mmu, base + OFF_MOVE3);

    uint32_t e0 = mmu_read_wram(mmu, base + OFF_EXP0);
    uint32_t e1 = mmu_read_wram(mmu, base + OFF_EXP1);
    uint32_t e2 = mmu_read_wram(mmu, base + OFF_EXP2);
    m->exp = (e0 << 16) | (e1 << 8) | e2;

    m->move_pp[0] = mmu_read_wram(mmu, base + OFF_PP0) & 0x3Fu;
    m->move_pp[1] = mmu_read_wram(mmu, base + OFF_PP1) & 0x3Fu;
    m->move_pp[2] = mmu_read_wram(mmu, base + OFF_PP2) & 0x3Fu;
    m->move_pp[3] = mmu_read_wram(mmu, base + OFF_PP3) & 0x3Fu;

    m->level  = mmu_read_wram(mmu, base + OFF_LEVEL);
    m->max_hp = ((uint16_t)mmu_read_wram(mmu, base + OFF_MAXHP_HI) << 8) |
                 mmu_read_wram(mmu, base + OFF_MAXHP_LO);
    m->atk    = ((uint16_t)mmu_read_wram(mmu, base + OFF_ATK_HI) << 8) |
                 mmu_read_wram(mmu, base + OFF_ATK_LO);
    m->def    = ((uint16_t)mmu_read_wram(mmu, base + OFF_DEF_HI) << 8) |
                 mmu_read_wram(mmu, base + OFF_DEF_LO);
    m->spd    = ((uint16_t)mmu_read_wram(mmu, base + OFF_SPD_HI) << 8) |
                 mmu_read_wram(mmu, base + OFF_SPD_LO);
    m->spc    = ((uint16_t)mmu_read_wram(mmu, base + OFF_SPC_HI) << 8) |
                 mmu_read_wram(mmu, base + OFF_SPC_LO);
  }

  for (int i = 0; i < out->party_count; i++) {
    uint8_t nick[11];
    for (int j = 0; j < 11; j++)
      nick[j] = mmu_read_wram(mmu, 0xD2B5u + (uint16_t)(i * 11 + j));
    p_string_convert(out->party[i].nickname, sizeof(out->party[i].nickname), 11, nick);
  }

  out->money     = decode_bcd(mmu, 0xD347, 3);
  out->badges    = mmu_read_wram(mmu, 0xD356);
  out->map_id    = mmu_read_wram(mmu, 0xD35E);
  out->dex_owned = (uint16_t)dex_count(mmu, 0xD2F7);
  out->dex_seen  = (uint16_t)dex_count(mmu, 0xD30A);

  out->in_battle     = mmu_read_wram(mmu, 0xD057);
  out->active_slot   = mmu_read_wram(mmu, 0xCC49);
  out->enemy_species = mmu_read_wram(mmu, 0xCFE5);
  out->enemy_level   = mmu_read_wram(mmu, 0xCFF3);
  out->enemy_type1   = mmu_read_wram(mmu, 0xCFEA);
  out->enemy_type2   = mmu_read_wram(mmu, 0xCFEB);
}
