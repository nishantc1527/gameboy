#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "internal.h"

int in[8];
uint16_t intr_loc[] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};

void req_intr(int intr) {
  uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
  set_bit(&val, intr);
  mmu_w_mem_raw(mmu, 0xFF0F, val);
}

void do_intr(int intr) {
  if (IME) {
    uint8_t val = mmu_r_mem_raw(mmu, 0xFF0F);
    clear_bit(&val, intr);
    mmu_w_mem_raw(mmu, 0xFF0F, val);
    push(PC);
    PC = intr_loc[intr];
  }
  HALT = 0;
  IME = 0;
}

void check_interrupt(void) {
  for (int intr = 0; intr < 5; intr++) {
    if (get_bit(IF, intr) && get_bit(IE, intr)) {
      do_intr(intr);
    }
  }
}

void intr_vblank_lcd(uint8_t stat, int prev_mode, int curr_mode) {
  int req_vblank = 0;
  int req_lcd = 0;
  if (prev_mode != curr_mode) {
    if (curr_mode == 1) req_vblank = 1;
    if (curr_mode == 0 && get_bit(stat, 3)) req_lcd = 1;
    if (curr_mode == 1 && get_bit(stat, 4)) req_lcd = 1;
    if (curr_mode == 2 && get_bit(stat, 5)) req_lcd = 1;
  }
  int prev_lyc = get_bit(stat, 2);
  int curr_lyc = LY == LYC;
  if (prev_lyc != curr_lyc && curr_lyc && get_bit(stat, 6)) req_lcd = 1;
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
