#include "gbemu/gbemu.h"

#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "rom_locs.h"

gbemu* gbemu_init(char* rom_name, int test_category,
                  uint8_t disassemble_enable) {
  gbemu* gb = malloc(sizeof(gbemu));
  gb->rom_name = rom_name;
  gb->test_category = test_category;
  gb->disassemble_enable = disassemble_enable;
  gb->b_done = 0;
  gb->cpu = init_cpu();
  gb->mmu = mmu_init(rom_name, BOOT_ROM_FILE, (int8_t)test_category);
  gb->apu = init_apu();
  gb->ppu = init_ppu();
  mmu_load(gb->mmu);
  return gb;
}

int gbemu_step_frame(gbemu* gb) {
  gb->ppu->frame = 0;
  while (!gb->ppu->frame) {
    const int cyc = step(gb->cpu, gb->mmu, gb->disassemble_enable,
                         gb->test_category, &gb->b_done);
    if (cyc == -1) return -1;
    gb->ppu->scn = (uint16_t)(gb->ppu->scn + cyc);
    if (gb->ppu->scn >= SCANLINE_LEN) {
      do_scanline(gb->ppu, gb->mmu);
      gb->ppu->scn -= SCANLINE_LEN;
    }
    update_lcd(gb->ppu, gb->mmu);
    update_timer(gb->cpu, gb->mmu, gb->apu, (uint8_t)cyc);
    check_dma(gb->mmu);
    upd_apu(gb->apu, gb->mmu);
    check_interrupt(gb->cpu, gb->mmu);
  }
  return 0;
}

void gbemu_free(gbemu* gb) {
  if (!gb) return;
  free(gb->cpu);
  free(gb->ppu);
  mmu_save(gb->mmu);
  mmu_free(gb->mmu);
  free(gb);
}
