#include "gbemu/joypad.h"

#include <stdlib.h>

#include "gbemu/bus.h"

#define REG_P1 0xFF00
#define P1_DPAD_SELECT 4
#define P1_BTN_SELECT 5
#define P1_RIGHT_A_BIT 0
#define P1_LEFT_B_BIT 1
#define P1_UP_SELECT_BIT 2
#define P1_DOWN_START_BIT 3
#define P1_UNUSED_BITS 0xC0

struct Joypad* joypad_init(void) {
  struct Joypad* j = calloc(1, sizeof(struct Joypad));
  j->select = BUS_OPEN_BUS;
  return j;
}

void joypad_free(struct Joypad* j) { free(j); }

uint8_t joypad_read(const struct Joypad* j) {
  uint8_t result = (j->select & 0x30) | P1_UNUSED_BITS | 0x0Fu;
  if ((j->select & (1u << P1_DPAD_SELECT)) == 0) {
    if (j->buttons[BTN_RIGHT]) result &= (uint8_t)~(1u << P1_RIGHT_A_BIT);
    if (j->buttons[BTN_LEFT]) result &= (uint8_t)~(1u << P1_LEFT_B_BIT);
    if (j->buttons[BTN_UP]) result &= (uint8_t)~(1u << P1_UP_SELECT_BIT);
    if (j->buttons[BTN_DOWN]) result &= (uint8_t)~(1u << P1_DOWN_START_BIT);
  }
  if ((j->select & (1u << P1_BTN_SELECT)) == 0) {
    if (j->buttons[BTN_A]) result &= (uint8_t)~(1u << P1_RIGHT_A_BIT);
    if (j->buttons[BTN_B]) result &= (uint8_t)~(1u << P1_LEFT_B_BIT);
    if (j->buttons[BTN_SELECT]) result &= (uint8_t)~(1u << P1_UP_SELECT_BIT);
    if (j->buttons[BTN_START]) result &= (uint8_t)~(1u << P1_DOWN_START_BIT);
  }
  return result;
}

void joypad_write(struct Joypad* j, uint8_t val) { j->select = val; }

void joypad_set_button(struct Joypad* j, int btn, uint8_t pressed,
                       struct Bus* bus) {
  uint8_t was_pressed = j->buttons[btn];
  j->buttons[btn] = pressed;
  if (pressed && !was_pressed) {
    uint8_t relevant = false;
    if ((j->select & (1u << P1_BTN_SELECT)) == 0 &&
        (btn == BTN_A || btn == BTN_B || btn == BTN_SELECT || btn == BTN_START))
      relevant = true;
    if ((j->select & (1u << P1_DPAD_SELECT)) == 0 &&
        (btn == BTN_RIGHT || btn == BTN_LEFT || btn == BTN_UP ||
         btn == BTN_DOWN))
      relevant = true;
    if (relevant) bus_req_intr(bus, INTR_JOYPAD);
  }
}
