#include "gbemu/ppu.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/util.h"

#define INTR_VBLANK 0
#define INTR_LCD 1

struct Ppu* init_ppu(void) {
  struct Ppu* ppu = malloc(sizeof(struct Ppu));
  ppu->WIN_CNT = 0;
  ppu->scn = 0;
  ppu->frame = 0;
  ppu->off_scn = 0;
  for (int i = 0; i < 8; i++) ppu->in[i] = 0;
  ppu->cgb_mode = 0;
  ppu->cgb_compat = 0;
  for (int y = 0; y < SCRN_HEIGHT; y++)
    for (int x = 0; x < SCRN_WIDTH; x++) ppu->cgb_dsp[y][x] = 0;
  return ppu;
}

static const uint16_t scx_mode3_penalty[8] = {0, 0, 0, 0, 4, 4, 4, 8};

void update_lcd(struct Ppu* ppu, struct Bus* bus) {
  uint8_t stat = bus_read(bus, 0xFF41);
  int prev_mode = stat & 0b11;
  uint8_t curr_mode;
  uint16_t mode3_end =
      (uint16_t)(252u + scx_mode3_penalty[bus_read(bus, 0xFF43) & 7u]);
  if (ppu->scn < 80)
    curr_mode = 2;
  else if (ppu->scn < mode3_end)
    curr_mode = 3;
  else
    curr_mode = 0;
  if (bus_read(bus, 0xFF44) >= SCRN_HEIGHT) curr_mode = 1;
  check_interrupt_vblank_lcd(bus, stat, prev_mode, curr_mode);
  if (prev_mode != 0 && curr_mode == 0) mmu_do_hdma_block(bus->mmu);
  stat &= (uint8_t)~(0b11);
  stat |= curr_mode;
  if (bus_read(bus, 0xFF44) == bus_read(bus, 0xFF45))
    set_bit(&stat, 2);
  else
    clear_bit(&stat, 2);
  bus_write(bus, 0xFF41, stat);
}
