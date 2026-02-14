#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct MMU MMU;

struct MMU *mmu_init(const char *rom_file_name,
                     const char *boot_rom_file_name,
                     uint8_t test_category);

uint8_t mmu_r_mem(const struct MMU *mmu, uint16_t loc);

uint8_t mmu_r_mem_raw(const struct MMU *mmu, uint16_t loc);

uint8_t mmu_r_ram_raw(const struct MMU *mmu, uint16_t loc);

void mmu_w_mem(struct MMU *mmu, uint16_t loc, uint8_t val);

void mmu_w_mem_raw(struct MMU *mmu, uint16_t loc, uint8_t val);

void mmu_w_ram_raw(struct MMU *mmu, uint16_t loc, uint8_t val);

const char *mmu_get_rom_title(const struct MMU *mmu);

void mmu_save(const struct MMU *mmu);

void mmu_load(struct MMU *mmu);

void mmu_free(struct MMU *mmu);
