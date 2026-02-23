#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

const uint32_t CPU_FREQ = 4194304;
const uint32_t DIV_FREQ = 16384;
const uint32_t TIM_FREQ_1 = 4096;
const uint32_t TIM_FREQ_2 = 262144;
const uint32_t TIM_FREQ_3 = 65536;
const uint32_t TIM_FREQ_4 = 16384;

uint8_t WIN_CNT;
uint16_t scn;
uint8_t frame;

struct CPU* init_cpu(void) {
  struct CPU* cpu = malloc(sizeof(struct CPU));
  cpu->PC = 0x0000;
  cpu->bHALT = 0;
  cpu->div_cnt = 0;
  cpu->tim_cnt = 0;
  cpu->intr_loc[0] = 0x0040;
  cpu->intr_loc[1] = 0x0048;
  cpu->intr_loc[2] = 0x0050;
  cpu->intr_loc[3] = 0x0058;
  cpu->intr_loc[4] = 0x0060;
  return cpu;
}

void update_timer(struct CPU* cpu, Mmu* mmu, uint8_t cycles) {
  uint8_t val = mmu_r_mem(mmu, TAC);
  switch (val & 0b11) {
    case 0b00: cpu->tim_thresh = TIM_FREQ_1; break;
    case 0b01: cpu->tim_thresh = TIM_FREQ_2; break;
    case 0b10: cpu->tim_thresh = TIM_FREQ_3; break;
    case 0b11: cpu->tim_thresh = TIM_FREQ_4; break;
  }
  cpu->tim_thresh = CPU_FREQ / cpu->tim_thresh;
  cpu->div_cnt = (uint32_t)(cpu->div_cnt + cycles);
  while (cpu->div_cnt >= CPU_FREQ / DIV_FREQ) {
    uint8_t div = mmu_r_mem(mmu, DIV);
    div++;
    mmu_w_mem_raw(mmu, 0xFF04, div);
    cpu->div_cnt -= CPU_FREQ / DIV_FREQ;
  }
  if (gb(mmu_r_mem(mmu, TAC), 2)) {
    cpu->tim_cnt = (uint32_t)(cpu->tim_cnt + cycles);
    while (cpu->tim_cnt >= cpu->tim_thresh) {
      uint8_t tima = mmu_r_mem(mmu, TIMA);
      check_interrupt_timer(mmu, tima);
      if (tima == 0xFF) tima = mmu_r_mem(mmu, TMA);
      else
        tima++;
      mmu_w_mem(mmu, 0xFF05, tima);
      cpu->tim_cnt -= cpu->tim_thresh;
    }
  }
}

void check_dma(Mmu* mmu) {
  if (mmu_r_mem(mmu, DMA) <= 0xDF) {
    uint16_t src = mmu_r_mem(mmu, DMA) * 0x100;
    for (uint16_t t = 0; t < 0xA0; t++) {
      mmu_w_mem(mmu, 0xFE00 + t, mmu_r_mem(mmu, src + t));
    }
    mmu_w_mem(mmu, 0xFF46, 0xFF);
  }
}
