#pragma once

#include <stdint.h>

#define TEST_BLARGG 0
#define TEST_MOONEYE 1

extern const uint32_t CPU_FREQ;
extern const uint32_t DIV_FREQ;
extern const uint32_t TIM_FREQ_1;
extern const uint32_t TIM_FREQ_2;
extern const uint32_t TIM_FREQ_3;
extern const uint32_t TIM_FREQ_4;

// Check interrupts
void check_interrupt(void);
void intr_vblank_lcd(uint8_t stat, int prev_mode, int curr_mode);
void intr_timer(uint8_t tima);
void intr_serial(void);
void intr_joypad(uint8_t prev_joyp, uint8_t curr_joyp);

// Initialize registers
void init_cpu(void);

// Update timer
void update_timer(uint8_t cycles);

// Perform DMA transfer
void check_dma(void);

// Fetch, decode, and execute CPU instruction
int step(void);
