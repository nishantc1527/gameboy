#include "gbemu/serial.h"

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
      uint8_t next = (uint8_t)((s->buf_tail + 1) % 0xFF);
      if (next != s->buf_head) {
        s->buf[s->buf_tail] = s->sb;
        s->buf_tail = next;
      }
      cpu->if_reg |= (uint8_t)(1u << INTR_SERIAL);
    } else {
      s->sc = val;
    }
    return;
  }
}

bool serial_has_byte(const struct Serial* s) {
  return s->buf_head != s->buf_tail;
}

uint8_t serial_take_byte(struct Serial* s) {
  if (s->buf_head == s->buf_tail) {
    s->byte_ready = false;
    return s->sb;
  }
  uint8_t byte = s->buf[s->buf_head];
  s->buf_head = (uint8_t)((s->buf_head + 1) % 0xFF);
  if (s->buf_head == s->buf_tail) s->byte_ready = false;
  return byte;
}
