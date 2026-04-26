#include "gbemu/serial.h"

#include <stdlib.h>

#include "gbemu/bus.h"

#define REG_SB 0xFF01
#define REG_SC 0xFF02
#define SC_TRANSFER_TRIGGER 0x81
#define SC_STORED_MASK 0x7F
#define SC_UNUSED_BITS 0x7E
#define SERIAL_BUF_SIZE 0xFF

struct Serial {
  uint8_t sb;
  uint8_t sc;
  uint8_t byte_ready;
  uint8_t buf[0xFF];
  uint8_t buf_head;
  uint8_t buf_tail;
};

struct Serial* serial_init(void) { return calloc(1, sizeof(struct Serial)); }

void serial_free(struct Serial* s) { free(s); }

uint8_t serial_read(const struct Serial* s, uint16_t addr) {
  if (addr == REG_SB) return s->sb;
  if (addr == REG_SC) return s->sc | SC_UNUSED_BITS;
  return BUS_OPEN_BUS;
}

void serial_write(struct Serial* s, uint16_t addr, uint8_t val,
                  struct Bus* bus) {
  if (addr == REG_SB) {
    s->sb = val;
    return;
  }
  if (addr == REG_SC) {
    if ((val & SC_TRANSFER_TRIGGER) == SC_TRANSFER_TRIGGER) {
      s->sc = val & SC_STORED_MASK;
      s->byte_ready = true;
      uint8_t next = (uint8_t)((s->buf_tail + 1) % SERIAL_BUF_SIZE);
      if (next != s->buf_head) {
        s->buf[s->buf_tail] = s->sb;
        s->buf_tail = next;
      }
      bus_req_intr(bus, INTR_SERIAL);
    } else {
      s->sc = val;
    }
    return;
  }
}

uint8_t serial_has_byte(const struct Serial* s) {
  return s->buf_head != s->buf_tail;
}

uint8_t serial_take_byte(struct Serial* s) {
  if (s->buf_head == s->buf_tail) {
    s->byte_ready = false;
    return s->sb;
  }
  uint8_t byte = s->buf[s->buf_head];
  s->buf_head = (uint8_t)((s->buf_head + 1) % SERIAL_BUF_SIZE);
  if (s->buf_head == s->buf_tail) s->byte_ready = false;
  return byte;
}
