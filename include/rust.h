#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
