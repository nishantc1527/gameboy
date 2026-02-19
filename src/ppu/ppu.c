#include "gbemu/ppu.h"

#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

void init_ppu(void) { WIN_CNT = 0; }

void update_lcd(void) {
  uint8_t stat = mmu_r_mem(mmu, LCD_STAT);
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  if (scn <= 80)
    curr_mode = 2;
  else if (scn <= 172)
    curr_mode = 3;  // TODO length of mode 3 can change
  else
    curr_mode = 0;
  if (mmu_r_mem(mmu, LY) >= SCRN_HEIGHT) curr_mode = 1;
  intr_vblank_lcd(stat, prev_mode, curr_mode);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC))
    st(&stat, 2);
  else
    cl(&stat, 2);
  mmu_w_mem(mmu, 0xFF41, stat);
}

int update_input(void) {
  uint8_t curr_joyp = mmu_r_mem(mmu, JOYP);
  curr_joyp |= 0xF;
  if (!gt(curr_joyp, 4)) {
    if (in[BTN_RIGHT]) cl(&curr_joyp, 0);
    if (in[BTN_LEFT]) cl(&curr_joyp, 1);
    if (in[BTN_UP]) cl(&curr_joyp, 2);
    if (in[BTN_DOWN]) cl(&curr_joyp, 3);
  }
  if (!gt(curr_joyp, 5)) {
    if (in[BTN_A]) cl(&curr_joyp, 0);
    if (in[BTN_B]) cl(&curr_joyp, 1);
    if (in[BTN_SELECT]) cl(&curr_joyp, 2);
    if (in[BTN_START]) cl(&curr_joyp, 3);
  }
  intr_joypad(mmu_r_mem(mmu, JOYP), curr_joyp);
  mmu_w_mem(mmu, 0xFF00, curr_joyp);
  return 0;
}
