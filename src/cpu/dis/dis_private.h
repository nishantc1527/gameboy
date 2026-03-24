#pragma once
#include <stdint.h>
struct Cpu;
struct Bus;
void dis_cb(struct Cpu* cpu, struct Bus* bus, uint8_t prfx);
void dis_00_3f(struct Cpu* cpu, struct Bus* bus, uint8_t instr);
void dis_40_7f(struct Cpu* cpu, struct Bus* bus, uint8_t instr);
void dis_80_bf(struct Cpu* cpu, struct Bus* bus, uint8_t instr);
void dis_c0_ff(struct Cpu* cpu, struct Bus* bus, uint8_t instr);
