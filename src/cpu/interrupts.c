#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"
#include "internal.h"

void do_intr(struct CPU* cpu, Mmu* mmu, uint8_t intr) {
  if (cpu->bIME) {
    uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
    cb(&val, intr);
    mmu_w_mem_raw(mmu, 0xFF0F, val);
    push(cpu, mmu, cpu->PC);
    cpu->PC = cpu->intr_loc[intr];
  }
  cpu->bHALT = 0;
  cpu->bIME = 0;
}

void check_interrupt(struct CPU* cpu, Mmu* mmu) {
  for (uint8_t intr = 0; intr < 5; intr++) {
    if (gb(mmu_r_mem(mmu, IF), intr) && gb(mmu_r_mem(mmu, IE), intr)) {
      do_intr(cpu, mmu, intr);
    }
  }
}

void check_interrupt_vblank_lcd(Mmu* mmu, uint8_t stat, int prev_mode,
                                int curr_mode) {
  int req_vblank = 0;
  int req_lcd = 0;
  if (prev_mode != curr_mode) {
    if (curr_mode == 1) req_vblank = 1;
    if (curr_mode == 0 && gb(stat, 3)) req_lcd = 1;
    if (curr_mode == 1 && gb(stat, 4)) req_lcd = 1;
    if (curr_mode == 2 && gb(stat, 5)) req_lcd = 1;
  }
  int prev_lyc = gb(stat, 2);
  int curr_lyc = mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC);
  if (prev_lyc != curr_lyc && curr_lyc && gb(stat, 6)) req_lcd = 1;
  if (req_vblank) req_intr(mmu, INTR_VBLANK);
  if (req_lcd) req_intr(mmu, INTR_LCD);
}

void check_interrupt_timer(Mmu* mmu, uint8_t tima) {
  if (tima == 0xFF) req_intr(mmu, INTR_TIMER);
}

void check_interrupt_serial(Mmu* mmu) {
  (void)mmu;
  // TODO
}

void check_interrupt_joypad(Mmu* mmu, uint8_t prev_joyp, uint8_t curr_joyp) {
  int req = 0;
  for (int i = 0; i < 4; i++) {
    int prev = (prev_joyp >> i) & 1;
    int curr = (curr_joyp >> i) & 1;
    if (prev != curr && curr == 0) req = 1;
  }
  if (req) req_intr(mmu, INTR_JOYPAD);
}
