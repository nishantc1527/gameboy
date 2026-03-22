#include "gbemu/joypad.h"

#include <stdlib.h>

#include "gbemu/mmu.h"

struct Joypad* joypad_init(void) {
  struct Joypad* j = calloc(1, sizeof(struct Joypad));
  j->select = 0xFF;
  return j;
}

void joypad_free(struct Joypad* j) { free(j); }

uint8_t joypad_read(const struct Joypad* j) {
  uint8_t result = (j->select & 0x30) | 0xCFu;
  if ((j->select & 0x10) == 0) {
    if (j->buttons[BTN_RIGHT]) result &= 0xFEu;
    if (j->buttons[BTN_LEFT]) result &= 0xFDu;
    if (j->buttons[BTN_UP]) result &= 0xFBu;
    if (j->buttons[BTN_DOWN]) result &= 0xF7u;
  }
  if ((j->select & 0x20) == 0) {
    if (j->buttons[BTN_A]) result &= 0xFEu;
    if (j->buttons[BTN_B]) result &= 0xFDu;
    if (j->buttons[BTN_SELECT]) result &= 0xFBu;
    if (j->buttons[BTN_START]) result &= 0xF7u;
  }
  return result;
}

void joypad_write(struct Joypad* j, uint8_t val) { j->select = val; }

void joypad_set_button(struct Joypad* j, int btn, uint8_t pressed,
                       struct Mmu* mmu) {
  uint8_t was_pressed = j->buttons[btn];
  j->buttons[btn] = pressed;
  if (pressed && !was_pressed) {
    uint8_t relevant = false;
    if ((j->select & 0x20) == 0 &&
        (btn == BTN_A || btn == BTN_B || btn == BTN_SELECT || btn == BTN_START))
      relevant = true;
    if ((j->select & 0x10) == 0 && (btn == BTN_RIGHT || btn == BTN_LEFT ||
                                    btn == BTN_UP || btn == BTN_DOWN))
      relevant = true;
    if (relevant) {
      uint8_t ifval = mmu_read_io(mmu, 0x0F);
      ifval |= 0x10u;  // INTR_JOYPAD = bit 4
      mmu_write_io(mmu, 0x0F, ifval);
    }
  }
}
