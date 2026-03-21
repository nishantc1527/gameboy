#include "gbemu/serial.h"

#include <stdio.h>
#include <stdlib.h>

#include "gbemu/cpu.h"

#define INTR_SERIAL 3

struct Serial* serial_init(void) { return calloc(1, sizeof(struct Serial)); }

void serial_free(struct Serial* s) { free(s); }

uint8_t serial_read(const struct Serial* s, uint16_t addr) {
  if (addr == 0xFF01) return s->sb;
  if (addr == 0xFF02) return s->sc | 0x7E;
  return 0xFF;
}

void serial_write(struct Serial* s, uint16_t addr, uint8_t val,
                  struct Cpu* cpu) {
  if (addr == 0xFF01) {
    s->sb = val;
    return;
  }
  if (addr == 0xFF02) {
    if ((val & 0x81) == 0x81) {
      s->sc = val & 0x7F;
      s->byte_ready = true;
      fprintf(stderr, "[DBG] serial byte: 0x%02X '%c'\n", s->sb,
              s->sb >= 0x20 ? s->sb : '?');
      cpu->if_reg |= (uint8_t)(1u << INTR_SERIAL);
    } else {
      s->sc = val;
    }
    return;
  }
}

uint8_t serial_take_byte(struct Serial* s) {
  s->byte_ready = false;
  return s->sb;
}
