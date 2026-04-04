#pragma once
#include <stdint.h>

struct Bus;
struct Serial;

struct Serial* serial_init(void);
void serial_free(struct Serial* s);
uint8_t serial_read(const struct Serial* s, uint16_t addr);
void serial_write(struct Serial* s, uint16_t addr, uint8_t val,
                  struct Bus* bus);
uint8_t serial_has_byte(const struct Serial* s);
uint8_t serial_take_byte(struct Serial* s);
