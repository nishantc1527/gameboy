#include "gbemu/ppu.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

struct Ppu* init_ppu(void) {
  struct Ppu* ppu = malloc(sizeof(struct Ppu));
  ppu->WIN_CNT = 0;
  ppu->scn = 0;
  ppu->frame = 0;
  for (int i = 0; i < 8; i++) ppu->in[i] = 0;
  return ppu;
}

void update_lcd(struct Ppu* ppu, Mmu* mmu) {
  uint8_t stat = mmu_r_mem(mmu, LCD_STAT);
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  if (ppu->scn <= 80) curr_mode = 2;
  else if (ppu->scn <= 172)
    curr_mode = 3;  // TODO length of mode 3 can change
  else
    curr_mode = 0;
  if (mmu_r_mem(mmu, LY) >= SCRN_HEIGHT) curr_mode = 1;
  check_interrupt_vblank_lcd(mmu, stat, prev_mode, curr_mode);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC)) set_bit(&stat, 2);
  else
    clear_bit(&stat, 2);
  mmu_w_mem(mmu, 0xFF41, stat);
}

int update_input(struct Ppu* ppu, Mmu* mmu) {
  uint8_t curr_joyp = mmu_r_mem(mmu, JOYP);
  curr_joyp |= 0xF;
  if (!get_bit(curr_joyp, 4)) {
    if (ppu->in[BTN_RIGHT]) clear_bit(&curr_joyp, 0);
    if (ppu->in[BTN_LEFT]) clear_bit(&curr_joyp, 1);
    if (ppu->in[BTN_UP]) clear_bit(&curr_joyp, 2);
    if (ppu->in[BTN_DOWN]) clear_bit(&curr_joyp, 3);
  }
  if (!get_bit(curr_joyp, 5)) {
    if (ppu->in[BTN_A]) clear_bit(&curr_joyp, 0);
    if (ppu->in[BTN_B]) clear_bit(&curr_joyp, 1);
    if (ppu->in[BTN_SELECT]) clear_bit(&curr_joyp, 2);
    if (ppu->in[BTN_START]) clear_bit(&curr_joyp, 3);
  }
  check_interrupt_joypad(mmu, mmu_r_mem(mmu, JOYP), curr_joyp);
  mmu_w_mem(mmu, 0xFF00, curr_joyp);
  return 0;
}
