#include "gbemu/serial.h"

#include <stdint.h>
#include <stdlib.h>

#include "gbemu/bus.h"

enum {
  REG_SB = 0xFF01,
  REG_SC = 0xFF02,
  SC_TRANSFER_TRIGGER = 0x81,
  SC_STORED_MASK = 0x7F,
  SC_UNUSED_BITS = 0x7E,
  SERIAL_BUF_SIZE = 0xFF
};

struct Serial {
  uint8_t sb;
  uint8_t sc;
  uint8_t byte_ready;
  uint8_t buf[0xFF];
  uint8_t buf_head;
  uint8_t buf_tail;
};

struct Serial* serial_init(void) { return calloc(1, sizeof(struct Serial)); }

void serial_free(struct Serial* ser) { free(ser); }

uint8_t serial_read(const struct Serial* ser, uint16_t addr) {
  if (addr == REG_SB) {
    return ser->sb;
  }
  if (addr == REG_SC) {
    return (uint8_t)((unsigned)ser->sc | SC_UNUSED_BITS);
  }
  return BUS_OPEN_BUS;
}

void serial_write(struct Serial* ser, uint16_t addr, uint8_t val,
                  struct Bus* bus) {
  if (addr == REG_SB) {
    ser->sb = val;
    return;
  }
  if (addr == REG_SC) {
    if (((unsigned)val & SC_TRANSFER_TRIGGER) == SC_TRANSFER_TRIGGER) {
      ser->sc = (uint8_t)((unsigned)val & SC_STORED_MASK);
      ser->byte_ready = 1U;
      uint8_t next = (uint8_t)((ser->buf_tail + 1) % SERIAL_BUF_SIZE);
      if (next != ser->buf_head) {
        ser->buf[ser->buf_tail] = ser->sb;
        ser->buf_tail = next;
      }
      bus_req_intr(bus, INTR_SERIAL);
    } else {
      ser->sc = val;
    }
    return;
  }
}

uint8_t serial_has_byte(const struct Serial* ser) {
  return ser->buf_head != ser->buf_tail;
}

uint8_t serial_take_byte(struct Serial* ser) {
  if (ser->buf_head == ser->buf_tail) {
    ser->byte_ready = 0U;
    return ser->sb;
  }
  uint8_t byte = ser->buf[ser->buf_head];
  ser->buf_head = (uint8_t)((ser->buf_head + 1) % SERIAL_BUF_SIZE);
  if (ser->buf_head == ser->buf_tail) {
    ser->byte_ready = 0U;
  }
  return byte;
}
