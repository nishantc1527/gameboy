#include "gbemu/ppu.h"

#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

int in[8];

void init_ppu(void) { WIN_CNT = 0; }

void update_lcd(struct CPU* cpu) {
  uint8_t stat = mmu_r_mem(mmu, LCD_STAT);
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  if (scn <= 80) curr_mode = 2;
  else if (scn <= 172)
    curr_mode = 3;  // TODO length of mode 3 can change
  else
    curr_mode = 0;
  if (mmu_r_mem(mmu, LY) >= SCRN_HEIGHT) curr_mode = 1;
  check_interrupt_vblank_lcd(stat, prev_mode, curr_mode);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC)) sb(&stat, 2);
  else
    cb(&stat, 2);
  mmu_w_mem(mmu, 0xFF41, stat);
}

int update_input(struct CPU* cpu) {
  uint8_t curr_joyp = mmu_r_mem(mmu, JOYP);
  curr_joyp |= 0xF;
  if (!gb(curr_joyp, 4)) {
    if (in[BTN_RIGHT]) cb(&curr_joyp, 0);
    if (in[BTN_LEFT]) cb(&curr_joyp, 1);
    if (in[BTN_UP]) cb(&curr_joyp, 2);
    if (in[BTN_DOWN]) cb(&curr_joyp, 3);
  }
  if (!gb(curr_joyp, 5)) {
    if (in[BTN_A]) cb(&curr_joyp, 0);
    if (in[BTN_B]) cb(&curr_joyp, 1);
    if (in[BTN_SELECT]) cb(&curr_joyp, 2);
    if (in[BTN_START]) cb(&curr_joyp, 3);
  }
  check_interrupt_joypad(mmu_r_mem(mmu, JOYP), curr_joyp);
  mmu_w_mem(mmu, 0xFF00, curr_joyp);
  return 0;
}
