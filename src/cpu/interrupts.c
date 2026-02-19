#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/util.h"
#include "internal.h"

int in[8];
uint16_t intr_loc[] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};

void do_intr(uint8_t intr) {
  if (bIME) {
    uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
    cl(&val, intr);
    mmu_w_mem_raw(mmu, 0xFF0F, val);
    push(PC);
    PC = intr_loc[intr];
  }
  bHALT = 0;
  bIME = 0;
}

void check_interrupt(void) {
  for (uint8_t intr = 0; intr < 5; intr++) {
    if (gt(mmu_r_mem(mmu, IF), intr) && gt(mmu_r_mem(mmu, IE), intr)) {
      do_intr(intr);
    }
  }
}

void intr_vblank_lcd(uint8_t stat, int prev_mode, int curr_mode) {
  int req_vblank = 0;
  int req_lcd = 0;
  if (prev_mode != curr_mode) {
    if (curr_mode == 1) req_vblank = 1;
    if (curr_mode == 0 && gt(stat, 3)) req_lcd = 1;
    if (curr_mode == 1 && gt(stat, 4)) req_lcd = 1;
    if (curr_mode == 2 && gt(stat, 5)) req_lcd = 1;
  }
  int prev_lyc = gt(stat, 2);
  int curr_lyc = mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC);
  if (prev_lyc != curr_lyc && curr_lyc && gt(stat, 6)) req_lcd = 1;
  if (req_vblank) req_intr(INTR_VBLANK);
  if (req_lcd) req_intr(INTR_LCD);
}

void intr_timer(uint8_t tima) {
  if (tima == 0xFF) req_intr(INTR_TIMER);
}

void intr_serial(void) {
  // TODO
}

void intr_joypad(uint8_t prev_joyp, uint8_t curr_joyp) {
  int req = 0;
  for (int i = 0; i < 4; i++) {
    int prev = (prev_joyp >> i) & 1;
    int curr = (curr_joyp >> i) & 1;
    if (prev != curr && curr == 0) req = 1;
  }
  if (req) req_intr(INTR_JOYPAD);
}
