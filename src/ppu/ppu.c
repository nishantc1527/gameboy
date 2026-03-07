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
  ppu->off_scn = 0;
  for (int i = 0; i < 8; i++) ppu->in[i] = 0;
  ppu->cgb_mode = 0;
  for (int y = 0; y < SCRN_HEIGHT; y++)
    for (int x = 0; x < SCRN_WIDTH; x++) ppu->cgb_dsp[y][x] = 0;
  return ppu;
}

static const uint16_t scx_mode3_penalty[8] = {0, 0, 0, 0, 4, 4, 4, 8};

void update_lcd(struct Ppu* ppu, Mmu* mmu) {
  uint8_t stat = mmu_r_mem(mmu, LCD_STAT);
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  uint16_t mode3_end =
      (uint16_t)(252 + scx_mode3_penalty[mmu_r_mem(mmu, SCX) & 7]);
  if (ppu->scn < 80)
    curr_mode = 2;
  else if (ppu->scn < mode3_end)
    curr_mode = 3;
  else
    curr_mode = 0;
  if (mmu_r_mem(mmu, LY) >= SCRN_HEIGHT) curr_mode = 1;
  check_interrupt_vblank_lcd(mmu, stat, prev_mode, curr_mode);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (mmu_r_mem(mmu, LY) == mmu_r_mem(mmu, LYC))
    set_bit(&stat, 2);
  else
    clear_bit(&stat, 2);
  mmu_w_mem(mmu, 0xFF41, stat);
}

int update_input(struct Ppu* ppu, Mmu* mmu) {
  uint8_t btns = (ppu->in[BTN_A] ? 0x01 : 0) | (ppu->in[BTN_B] ? 0x02 : 0) |
                 (ppu->in[BTN_SELECT] ? 0x04 : 0) |
                 (ppu->in[BTN_START] ? 0x08 : 0);
  uint8_t dirs = (ppu->in[BTN_RIGHT] ? 0x01 : 0) |
                 (ppu->in[BTN_LEFT] ? 0x02 : 0) | (ppu->in[BTN_UP] ? 0x04 : 0) |
                 (ppu->in[BTN_DOWN] ? 0x08 : 0);
  mmu_set_joypad(mmu, btns, dirs);
  return 0;
}
