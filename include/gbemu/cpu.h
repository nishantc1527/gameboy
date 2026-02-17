#pragma once

#include <stdint.h>

#define JOYP mmu_r_mem(mmu, 0xFF00)
#define SB mmu_r_mem(mmu, 0xFF01)
#define SC mmu_r_mem(mmu, 0xFF02)
#define DIV mmu_r_mem(mmu, 0xFF04)
#define TIMA mmu_r_mem(mmu, 0xFF05)
#define TMA mmu_r_mem(mmu, 0xFF06)
#define TAC mmu_r_mem(mmu, 0xFF07)
#define IF mmu_r_mem(mmu, 0xFF0F)
#define IE mmu_r_mem(mmu, 0xFFFF)

#define TEST_BLARGG 0
#define TEST_MOONEYE 1

extern const uint32_t CPU_FREQ;
extern const uint32_t DIV_FREQ;
extern const uint32_t TIM_FREQ_1;
extern const uint32_t TIM_FREQ_2;
extern const uint32_t TIM_FREQ_3;
extern const uint32_t TIM_FREQ_4;

extern uint8_t HALT, IME;
extern uint8_t A, B, C, D, E, F, H, L;
extern uint16_t PC, SP;
extern uint32_t tim_thresh;
extern uint8_t done;

extern char* rom_name;
extern int test_category;
extern uint8_t disassemble;

static inline uint8_t get_bit(uint8_t var, uint8_t bt) {
  return (var >> bt) & 1;
}
static inline void set_bit(uint8_t* var, uint8_t bt) {
  *var |= (uint8_t)(1 << bt);
}
static inline void clear_bit(uint8_t* var, uint8_t bt) {
  *var &= (uint8_t)~(1 << bt);
}
static inline uint8_t gt_flg(uint8_t flg) { return get_bit(F, flg); }

// Check interrupts
void check_interrupt(void);
void intr_vblank_lcd(uint8_t stat, int prev_mode, int curr_mode);
void intr_timer(uint8_t tima);
void intr_serial(void);
void intr_joypad(uint8_t prev_joyp, uint8_t curr_joyp);

// Initialize registers
void init_reg(void);

// Update timer
void update_timer(uint8_t cycles);

// Perform DMA transfer
void check_dma(void);

// Fetch, decode, and execute CPU instruction
int exec_instr(void);

uint8_t rd8(void);
uint16_t rd16(void);

void push(uint16_t val);
uint16_t pop(void);
uint16_t pk(void);

void kp(void);
