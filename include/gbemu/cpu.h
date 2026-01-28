#pragma once

#include <stdint.h>

#define JOYP r_mem(0xFF00)
#define SB r_mem(0xFF01)
#define SC r_mem(0xFF02)
#define DIV r_mem(0xFF04)
#define TIMA r_mem(0xFF05)
#define TMA r_mem(0xFF06)
#define TAC r_mem(0xFF07)
#define IF r_mem(0xFF0F)
#define IE r_mem(0xFFFF)

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

extern char* rom_name;
extern uint8_t test_category;
extern uint8_t disassemble;

static inline uint8_t get_bit(uint8_t var, uint8_t bt) {
  return (var >> bt) & 1;
}
static inline void set_bit(uint8_t* var, uint8_t bt) { *var |= (1 << bt); }
static inline void clear_bit(uint8_t* var, uint8_t bt) { *var &= ~(1 << bt); }
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
void update_timer(int cycles);

// Perform DMA transfer
void check_dma(void);

// Fetch, decode, and execute CPU instruction
int exec_instr(void);
