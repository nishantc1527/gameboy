#pragma once

#include <stdint.h>

extern char rom_title[20];
extern uint8_t cart_type;
extern uint8_t rom_size, ram_size;

extern uint8_t mem[0x10000];
extern uint8_t brom[0x100];
extern uint8_t rom[0x800000];
extern uint8_t extern_ram[0x20000];

extern char* test_out;
extern int test_out_idx;

void init_mem(void);

int save(void);
int load(void);

int get_rom_info(void);

uint8_t r_mem(uint16_t loc);
void w_mem(uint16_t loc, uint8_t val);

uint8_t rd8(void);
uint16_t rd16(void);

void push(uint16_t val);
uint16_t pop(void);
uint16_t pk(void);

void kp(void);
