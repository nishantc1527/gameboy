#pragma once

#include <stdint.h>

#include "gbemu/apu.h"
#include "gbemu/mmu.h"

extern const uint32_t CPU_FREQ;
extern const uint32_t DIV_FREQ;
extern const uint32_t TIM_FREQ_1;
extern const uint32_t TIM_FREQ_2;
extern const uint32_t TIM_FREQ_3;
extern const uint32_t TIM_FREQ_4;

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  uint8_t bHALT, bIME, bIME_pending, bHALT_BUG, tima_overflow_pending;
  uint32_t tim_cnt, tim_thresh, div_cnt;
  uint16_t intr_loc[5];
  uint8_t cyc_ext;
  uint8_t cgb_mode;
};

struct Cpu* init_cpu(void);
void post_boot_cpu(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
int step(struct Cpu* cpu, Mmu* mmu, struct Apu* apu, uint8_t disassemble_enable,
         int test_category, uint8_t* b_done, const uint16_t* watch_addrs,
         uint8_t watch_count, uint64_t total_cycles);

int check_interrupt(struct Cpu* cpu, Mmu* mmu);
void check_interrupt_vblank_lcd(Mmu* mmu, uint8_t stat, int prev_mode,
                                int curr_mode);
void check_interrupt_timer(Mmu* mmu, uint8_t tima);
void check_interrupt_serial(Mmu* mmu);
void check_interrupt_joypad(Mmu* mmu, uint8_t prev_joyp, uint8_t curr_joyp);

void update_timer(struct Cpu* cpu, Mmu* mmu, struct Apu* apu, uint8_t cycles);
void check_dma(Mmu* mmu);
