#pragma once
#include <stdint.h>

#define BTN_A 0
#define BTN_B 1
#define BTN_START 2
#define BTN_SELECT 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_LEFT 6
#define BTN_RIGHT 7

struct Cpu;

struct Joypad {
  uint8_t buttons[8];
  uint8_t select;
};

struct Joypad* joypad_init(void);
void joypad_free(struct Joypad* j);
uint8_t joypad_read(const struct Joypad* j);
void joypad_write(struct Joypad* j, uint8_t val);
void joypad_set_button(struct Joypad* j, int btn, uint8_t pressed,
                       struct Cpu* cpu);
