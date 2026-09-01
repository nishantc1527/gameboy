# Pokémon Mode — Implementation Plan

## Context

The emulator already detects Pokémon Red/Blue (`p_check_pokemon`), initializes save checksums, and decodes Gen I strings (`p_string_convert`, `p_table`). The goal is a full "Pokémon mode": when a Gen I Pokémon game is loaded, the SDL window transforms into a companion hub — game screen in the center, party and information framing it on all sides.

---

## Finalized Layout

```
┌──────────────────────────────────────────────────────────────┐
│  POKÉMON RED              Session: 01:23:45         DMG Mode  │  ← header (always)
├───────────────┬─────────────────────────────┬────────────────┤
│  [Mon1][Mon2] │                             │  Right panel   │
│  [Mon3][Mon4] │       GAME SCREEN           │  (see below)   │
│  [Mon5][Mon6] │       (centered)            │                │
├───────────────┴─────────────────────────────┴────────────────┤
│  Bottom strip (see below)                                      │
└──────────────────────────────────────────────────────────────┘
```

**Header** (always visible):
- Game title: "POKÉMON RED" or "POKÉMON BLUE" (large, styled in game color)
- Session time (wall-clock timer, no WRAM needed)
- Hardware mode: "DMG" or "CGB"

**Left panel** (always visible — 3×2 party grid):
- 6 slots arranged 3 rows × 2 columns, matching the in-game party screen
- Each card: species sprite, colored status border, HP bar, nickname, level
- Status border colors: white = healthy, purple = poisoned, orange = burned, cyan = frozen, yellow = paralyzed, gray = fainted/asleep
- HP bar: green > 50%, yellow 25–50%, red < 25%
- Clicking a card sets `pokemon_focused_slot` (used by bottom strip)

**Right panel** (context-dependent):
- **Overworld**: trainer name, money (BCD-decoded), 8 badge indicators, current location, Pokédex seen + owned count
- **Battle**: full detail on the active Pokémon — species name, nickname, level, type(s), HP bar, ATK/DEF/SPD/SPC stats, all 4 moves with PP

**Bottom strip** (context-dependent):
- **Overworld**: 4 moves + PP for `pokemon_focused_slot` (defaults to slot 0); updates when a party card is clicked
- **Battle**: enemy Pokémon — species name, level, type(s)

---

## Game State Detection

Single WRAM read each frame:
```c
uint8_t in_battle = mmu_read_wram(mmu, 0xD057);
// 0 = overworld, 1 = wild battle, 2 = trainer battle
```

Active party slot in battle (which card to highlight on the left):
```c
uint8_t active_slot = mmu_read_wram(mmu, 0xCC49); // wPlayerBattleMonPartyPos
```

Enemy species in battle:
```c
uint8_t enemy_species = mmu_read_wram(mmu, 0xCFE5); // wEnemyMonSpecies
uint8_t enemy_level   = mmu_read_wram(mmu, 0xCFF3); // wEnemyMonLevel
uint8_t enemy_type1   = mmu_read_wram(mmu, 0xCFEA);
uint8_t enemy_type2   = mmu_read_wram(mmu, 0xCFEB);
```

*Note: verify all WRAM addresses against the Bulbapedia HTML at `src/pokemon/` and the Gen I disassembly before use.*

---

## Data Source: WRAM (live)

ERAM is the save file — the game only flushes WRAM → ERAM on explicit save. Party HP and status update continuously in WRAM. Always read from WRAM for live data.

**Key WRAM addresses (Pokémon Red/Blue, full GB bus addresses):**
```
0xD158  Player name         (11 bytes, Gen I encoding)
0xD163  Party count         (1 byte)
0xD164  Party species array (7 bytes, 0xFF-terminated)
0xD16B  Party mon 1         (44 bytes; stride 44 per slot → mon N at 0xD16B + N*44)
0xD273  OT names            (6 × 11 bytes)
0xD2B5  Party nicknames     (6 × 11 bytes)
0xD2F7  Pokédex owned       (19 bytes bitset)
0xD30A  Pokédex seen        (19 bytes bitset)
0xD347  Money               (3 bytes, BCD big-endian)
0xD356  Badges              (1 byte, bit 0 = Boulder … bit 7 = Earth)
0xD35E  Current map ID      (1 byte → location name lookup)
```

**Per-mon offsets within the 44-byte party entry (base = 0xD16B + slot*44):**
```
+0x00        Species (internal ID)
+0x01–0x02   Current HP (big-endian uint16)
+0x04        Status condition
+0x05–0x06   Type 1, Type 2
+0x08–0x0B   Move IDs (4 bytes)
+0x0E–0x10   Experience (3 bytes, big-endian)
+0x21–0x24   Move PP (4 bytes; lower 6 bits = current PP)
+0x25        Level
+0x26–0x27   Max HP (big-endian uint16)
+0x28–0x29   Attack
+0x2A–0x2B   Defense
+0x2C–0x2D   Speed
+0x2E–0x2F   Special
```

---

## Sprites

**Strategy:** download a Gen I sprite sheet PNG (151 mons in National Dex order), bake into the executable at build time, decode at runtime with `stb_image.h`.

**Build step** (added to Makefile):
```makefile
src/pokemon/sprites_data.c: assets/sprites.png
    xxd -i $< > $@
```

**Runtime loading:**
- Add `stb_image.h` to `vendor/` (single-header, no dependencies)
- `p_load_sprites(SDL_Renderer*)` — decodes the embedded PNG from memory → uploads to one `SDL_Texture*` atlas
- `p_get_sprite_rect(uint8_t species_id, SDL_FRect* out)` — maps internal species ID → UV rect in the atlas via a 256-entry lookup table (internal ID → National Dex number → sprite sheet row/col)
- Atlas texture freed on `pokemon_cleanup()`

---

## Data Structures (new, in `include/gbemu/pokemon.h`)

```c
typedef struct {
    uint8_t  species;         // internal species ID (0 = empty slot)
    char     nickname[12];    // decoded via p_string_convert
    uint8_t  level;
    uint16_t hp, max_hp;
    uint8_t  status;
    uint8_t  type1, type2;
    uint8_t  move_ids[4];
    uint8_t  move_pp[4];      // current PP (lower 6 bits)
    uint16_t atk, def, spd, spc;
    uint32_t exp;
} PokemonMon;

typedef struct {
    char       trainer_name[12];
    uint8_t    party_count;
    PokemonMon party[6];
    uint8_t    badges;
    uint32_t   money;
    uint16_t   dex_seen, dex_owned;
    uint8_t    map_id;
    uint8_t    in_battle;     // 0/1/2
    uint8_t    active_slot;
    uint8_t    enemy_species, enemy_level, enemy_type1, enemy_type2;
} PokemonPlayer;
```

---

## Implementation Steps

### 1. Data reader — `src/pokemon/party.c`
- `void p_read_player(struct Mmu* mmu, PokemonPlayer* out)` — populates everything above from WRAM
- `uint32_t p_decode_bcd(struct Mmu* mmu, uint16_t addr, int n)` — BCD money decoder
- `int p_dex_count(struct Mmu* mmu, uint16_t addr)` — popcount over 19-byte bitset
- Reuse: `p_string_convert()` + `p_table[]` from `src/pokemon/util.c`

### 2. Lookup tables — `src/pokemon/tables.c`
Static arrays (all `const char* const`):
- `pokemon_species_names[256]` — internal ID → display name, `NULL` for gaps
- `pokemon_move_names[166]` — move ID → name
- `pokemon_type_names[15]` — type ID → name
- `pokemon_type_colors[15]` — `ImVec4` per type (standard type palette)
- `pokemon_map_names[256]` — map ID → location string
- `pokemon_internal_to_dex[256]` — internal species ID → National Dex number (for sprite indexing)

### 3. Sprites — `src/pokemon/sprites.c` + `assets/sprites.png`
- Download sprite sheet into `assets/sprites.png`
- Makefile bake step generates `src/pokemon/sprites_data.c`
- `void p_load_sprites(SDL_Renderer* rnd)` — decode + upload atlas
- `void p_get_sprite_rect(uint8_t species_id, SDL_FRect* out)`
- `void p_cleanup_sprites(void)` — destroy atlas texture

### 4. Window layout — `sdl/sdl.c`, `sdl/render.c`, `sdl/init.c`

Define layout constants:
```c
#define POKEMON_HEADER_H  50
#define POKEMON_BOTTOM_H  70
#define POKEMON_LEFT_W   220   // party panel
#define POKEMON_RIGHT_W  260   // info/detail panel
```

- `sdl/sdl.c` `load_rom()`: after `pokemon_enabled` is set, call `pokemon_resize_window()` to expand
- `pokemon_resize_window()` / `pokemon_restore_window()` — `SDL_SetWindowSize()` calls
- `sdl/render.c`: when `pokemon_enabled`, offset game render to `(POKEMON_LEFT_W, POKEMON_HEADER_H)` instead of centering on full window

### 5. ImGui overlay — `sdl/imgui_impl.cpp`

New functions:
- `draw_pokemon_header(AppState*, PokemonPlayer*)` — top bar
- `draw_pokemon_party(AppState*, PokemonPlayer*)` — left 3×2 grid; handles click → sets `focused_slot`
- `draw_pokemon_right_overworld(AppState*, PokemonPlayer*)` — trainer info
- `draw_pokemon_right_battle(AppState*, PokemonPlayer*)` — active mon details
- `draw_pokemon_bottom_overworld(AppState*, PokemonPlayer*)` — focused mon moves
- `draw_pokemon_bottom_battle(AppState*, PokemonPlayer*)` — enemy info
- `draw_pokemon_overlay(AppState*)` — reads player state each frame, dispatches to the above

Add `int pokemon_focused_slot` to `AppState` (in `include/gbemu/sdl.h`).

Called from `draw_ui()`:
```cpp
if (pokemon_enabled && state->gb) draw_pokemon_overlay(state);
```

### 6. Makefile
- Add `src/pokemon/party.c`, `src/pokemon/tables.c`, `src/pokemon/sprites.c`, `src/pokemon/sprites_data.c` to `SRCS`
- Add sprite bake rule (depends on `assets/sprites.png`)

---

## Files Modified / Created

| File | Change |
|------|--------|
| `include/gbemu/pokemon.h` | Add `PokemonMon`, `PokemonPlayer`, new fn decls |
| `include/gbemu/sdl.h` | Add `pokemon_focused_slot` to `AppState` |
| `src/pokemon/party.c` | New — WRAM reader |
| `src/pokemon/tables.c` | New — name/color lookup tables |
| `src/pokemon/sprites.c` | New — sprite atlas load/query |
| `src/pokemon/sprites_data.c` | Generated — `xxd -i` output |
| `assets/sprites.png` | New — downloaded Gen I sprite sheet |
| `vendor/stb_image.h` | New — single-header PNG decoder |
| `sdl/imgui_impl.cpp` | Add all `draw_pokemon_*` functions |
| `sdl/render.c` | Conditional game offset when `pokemon_enabled` |
| `sdl/sdl.c` | Window resize on pokemon ROM load/close |
| `Makefile` | New sources + sprite bake step |

No Rust changes needed — `mmu_read_wram` already exists and is exported.

---

## Save Editing (future, not v1)

`mmu_write_wram()` already exists in the Rust FFI. Entry point: right-click context menu on a party card. Works cleanly out-of-battle; mid-battle requires writing to the battle-mon structure at a separate WRAM location.

---

## Verification

1. `make gbemu` — clean build
2. Load Pokémon Red/Blue → window expands to hub layout, header shows game title
3. Walk around in overworld → left party updates live, right shows trainer info, bottom shows focused mon's moves, clicking a party card changes the bottom strip
4. Enter a battle → right panel switches to active mon details, bottom shows enemy species/level/type, active party slot is highlighted
5. Load a non-Pokémon ROM → normal window, no panels
6. Close ROM → window restores to original size
