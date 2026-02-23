#pragma once

#include <stdint.h>

extern const uint32_t CPU_FREQ;
extern const uint32_t DIV_FREQ;
extern const uint32_t TIM_FREQ_1;
extern const uint32_t TIM_FREQ_2;
extern const uint32_t TIM_FREQ_3;
extern const uint32_t TIM_FREQ_4;

struct CPU {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  uint8_t bHALT, bIME;
  uint32_t tim_cnt, tim_thresh, div_cnt;
  uint16_t intr_loc[5];
};

// Check interrupts
void check_interrupt(struct CPU* cpu);
void check_interrupt_vblank_lcd(uint8_t stat, int prev_mode, int curr_mode);
void check_interrupt_timer(uint8_t tima);
void check_interrupt_serial(void);
void check_interrupt_joypad(uint8_t prev_joyp, uint8_t curr_joyp);

struct CPU* init_cpu(void);

void update_timer(struct CPU* cpu, uint8_t cycles);
void check_dma(void);
int step(struct CPU* cpu);
