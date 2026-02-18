#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "internal.h"

const uint32_t CPU_FREQ = 4194304;
const uint32_t DIV_FREQ = 16384;
const uint32_t TIM_FREQ_1 = 4096;
const uint32_t TIM_FREQ_2 = 262144;
const uint32_t TIM_FREQ_3 = 65536;
const uint32_t TIM_FREQ_4 = 16384;

uint16_t PC, SP;
uint8_t A, B, C, D, E, F, H, L;
uint8_t IME, WIN_CNT, HALT;
uint16_t scn;
uint32_t tim_cnt, tim_thresh, div_cnt;
uint8_t frame;

void init_reg(void) {
  PC = 0x0000;
  HALT = 0;
  div_cnt = 0;
  tim_cnt = 0;
}

uint16_t gt_AF(void) { return (uint16_t)(((uint16_t)A) << 8) | (uint16_t)F; }

void st_AF(uint16_t AF) {
  A = (uint8_t)(AF >> 8);
  F = (uint8_t)AF;
}

uint16_t gt_BC(void) { return (uint16_t)(((uint16_t)B) << 8) | (uint16_t)C; }

void st_BC(uint16_t BC) {
  B = (uint8_t)(BC >> 8);
  C = (uint8_t)BC;
}

uint16_t gt_DE(void) { return (uint16_t)(((uint16_t)D) << 8) | (uint16_t)E; }

void st_DE(uint16_t DE) {
  D = (uint8_t)(DE >> 8);
  E = (uint8_t)DE;
}

uint16_t gt_HL(void) { return (uint16_t)(((uint16_t)H) << 8) | (uint16_t)L; }

void st_HL(uint16_t HL) {
  H = (uint8_t)(HL >> 8);
  L = (uint8_t)HL;
}

void update_timer(uint8_t cycles) {
  uint8_t val = mmu_r_mem(mmu, TAC);
  switch (val & 0b11) {
    case 0b00:
      tim_thresh = TIM_FREQ_1;
      break;
    case 0b01:
      tim_thresh = TIM_FREQ_2;
      break;
    case 0b10:
      tim_thresh = TIM_FREQ_3;
      break;
    case 0b11:
      tim_thresh = TIM_FREQ_4;
      break;
  }
  tim_thresh = CPU_FREQ / tim_thresh;
  div_cnt = (uint32_t)(div_cnt + cycles);
  while (div_cnt >= CPU_FREQ / DIV_FREQ) {
    uint8_t div = mmu_r_mem(mmu, DIV);
    div++;
    mmu_w_mem_raw(mmu, 0xFF04, div);
    div_cnt -= CPU_FREQ / DIV_FREQ;
  }
  if (get_bit(mmu_r_mem(mmu, TAC), 2)) {
    tim_cnt = (uint32_t)(tim_cnt + cycles);
    while (tim_cnt >= tim_thresh) {
      uint8_t tima = mmu_r_mem(mmu, TIMA);
      intr_timer(tima);
      if (tima == 0xFF)
        tima = mmu_r_mem(mmu, TMA);
      else
        tima++;
      mmu_w_mem(mmu, 0xFF05, tima);
      tim_cnt -= tim_thresh;
    }
  }
}

void check_dma(void) {
  if (DMA <= 0xDF) {
    uint16_t src = DMA * 0x100;
    for (uint16_t t = 0; t < 0xA0; t++) {
      mmu_w_mem(mmu, 0xFE00 + t, mmu_r_mem(mmu, src + t));
    }
    mmu_w_mem(mmu, 0xFF46, 0xFF);
  }
}
