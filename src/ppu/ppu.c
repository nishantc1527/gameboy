#include "gbemu/ppu.h"

#include <stdint.h>

#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "internal.h"

void init_ppu(void) { WIN_CNT = 0; }

void update_lcd(void) {
  uint8_t stat = LCD_STAT;
  int prev_mode = stat & 0b11;
  int curr_mode;
  if (scn <= 80)
    curr_mode = 2;
  else if (scn <= 172)
    curr_mode = 3;  // TODO length of mode 3 can change
  else
    curr_mode = 0;
  if (LY >= SCRN_HEIGHT) curr_mode = 1;
  intr_vblank_lcd(stat, prev_mode, curr_mode);
  stat &= ~(0b11);
  stat |= curr_mode;
  if (LY == LYC)
    set_bit(&stat, 2);
  else
    clear_bit(&stat, 2);
  w_mem(0xFF41, stat);
}

int update_input(void) {
  uint8_t curr_joyp = JOYP;
  curr_joyp |= 0xF;
  if (!get_bit(curr_joyp, 4)) {
    if (in[BTN_RIGHT]) clear_bit(&curr_joyp, 0);
    if (in[BTN_LEFT]) clear_bit(&curr_joyp, 1);
    if (in[BTN_UP]) clear_bit(&curr_joyp, 2);
    if (in[BTN_DOWN]) clear_bit(&curr_joyp, 3);
  }
  if (!get_bit(curr_joyp, 5)) {
    if (in[BTN_A]) clear_bit(&curr_joyp, 0);
    if (in[BTN_B]) clear_bit(&curr_joyp, 1);
    if (in[BTN_SELECT]) clear_bit(&curr_joyp, 2);
    if (in[BTN_START]) clear_bit(&curr_joyp, 3);
  }
  intr_joypad(JOYP, curr_joyp);
  w_mem(0xFF00, curr_joyp);
  return 0;
}
