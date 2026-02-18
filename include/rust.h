#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define NR10 65296

#define NR11 65297

#define NR12 65298

#define NR13 65299

#define NR14 65300

#define NR21 65302

#define NR22 65303

#define NR23 65304

#define NR24 65305

#define NR30 65306

#define NR31 65307

#define NR32 65308

#define NR33 65309

#define NR34 65310

#define NR41 65312

#define NR42 65313

#define NR43 65314

#define NR44 65315

#define NR50 65316

#define NR51 65317

#define NR52 65318

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
