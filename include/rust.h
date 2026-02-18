#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define JOYP 65280

#define SB 65281

#define SC 65282

#define DIV 65284

#define TIMA 65285

#define TMA 65286

#define TAC 65287

#define IF 65295

#define IE 65535

#define LCDC 65344

#define LCD_STAT 65345

#define SCY 65346

#define SCX 65347

#define LY 65348

#define LYC 65349

#define DMA 65350

#define BGP 65351

#define OBP0 65352

#define OBP1 65353

#define WY 65354

#define WX 65355

typedef struct Mmu Mmu;

struct Mmu *mmu_init(const char *rom_file_name,
                     const char *boot_rom_file_name,
                     int8_t test_category);

uint8_t mmu_r_mem(const struct Mmu *mmu, uint16_t loc);

uint8_t mmu_r_mem_raw(const struct Mmu *mmu, uint16_t loc);

uint8_t mmu_r_ram_raw(const struct Mmu *mmu, uint16_t loc);

void mmu_w_mem(struct Mmu *mmu, uint16_t loc, uint8_t val);

void mmu_w_mem_raw(struct Mmu *mmu, uint16_t loc, uint8_t val);

void mmu_w_ram_raw(struct Mmu *mmu, uint16_t loc, uint8_t val);

const char *mmu_get_rom_title(const struct Mmu *mmu);

void mmu_save(const struct Mmu *mmu);

void mmu_load(struct Mmu *mmu);

void mmu_free(struct Mmu *mmu);
