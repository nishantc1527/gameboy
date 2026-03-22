#pragma once
#include <stdint.h>

struct Cpu;

struct Serial {
  uint8_t sb;
  uint8_t sc;
  uint8_t byte_ready;
  uint8_t buf[0xFF];
  uint8_t buf_head;
  uint8_t buf_tail;
};

struct Serial* serial_init(void);
void serial_free(struct Serial* s);
uint8_t serial_read(const struct Serial* s, uint16_t addr);
void serial_write(struct Serial* s, uint16_t addr, uint8_t val,
                  struct Cpu* cpu);
uint8_t serial_has_byte(const struct Serial* s);
uint8_t serial_take_byte(struct Serial* s);
