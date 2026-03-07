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

typedef enum TestCategory {
  TestAge,
  TestBlarggCpu,
  TestBlarggAudio,
  TestBlarggCpuTime,
  TestBlarggMemTime,
  TestBlarggHaltBug,
  TestBlarggInterruptTime,
  TestBlarggMemTime2,
  TestBlarggOamBug,
  TestBlarggCgbSound,
  TestBully,
  TestAcid2,
  TestGambatte,
  TestMicro,
  TestLittle,
  TestMbc3,
  TestMealybug,
  TestMooneye,
  TestSame,
  TestRtc3Basic,
  TestRtc3Range,
  TestRtc3Sub,
  TestScribble,
  TestStrike,
  TestTurtle,
} TestCategory;

typedef struct Mmu Mmu;

extern const uint16_t JOYP;

extern const uint16_t SB;

extern const uint16_t SC;

extern const uint16_t DIV;

extern const uint16_t TIMA;

extern const uint16_t TMA;

extern const uint16_t TAC;

extern const uint16_t IF;

extern const uint16_t IE;

extern const uint16_t LCDC;

extern const uint16_t LCD_STAT;

extern const uint16_t SCY;

extern const uint16_t SCX;

extern const uint16_t LY;

extern const uint16_t LYC;

extern const uint16_t DMA;

extern const uint16_t BGP;

extern const uint16_t OBP0;

extern const uint16_t OBP1;

extern const uint16_t WY;

extern const uint16_t WX;

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

bool mmu_is_cgb(const struct Mmu *mmu);

bool mmu_boot_skipped(const struct Mmu *mmu);

bool mmu_take_div_reset(struct Mmu *mmu);

uint8_t mmu_get_vram_bank1_byte(const struct Mmu *mmu, uint16_t addr);

uint8_t mmu_get_bg_pal_byte(const struct Mmu *mmu, uint8_t idx);

uint8_t mmu_get_obj_pal_byte(const struct Mmu *mmu, uint8_t idx);

void mmu_set_joypad(struct Mmu *mmu, uint8_t btns, uint8_t dirs);

void mmu_advance_rtc(struct Mmu *mmu, uint64_t cycles);

void mmu_free(struct Mmu *mmu);
