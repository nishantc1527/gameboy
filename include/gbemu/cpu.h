#pragma once
#include <stdbool.h>
#include <stdint.h>

struct Bus;
struct Cpu;

struct Cpu* cpu_init(bool cgb_mode);
uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_read_if(const struct Cpu* cpu);
void cpu_write_if(struct Cpu* cpu, uint8_t val);
void cpu_req_intr(struct Cpu* cpu, uint8_t intr);
uint8_t cpu_read_ie(const struct Cpu* cpu);
void cpu_write_ie(struct Cpu* cpu, uint8_t val);
bool cpu_get_cgb_mode(const struct Cpu* cpu);
bool cpu_check_ld_b_b(struct Cpu* cpu);
uint8_t cpu_get_b(const struct Cpu* cpu);
uint8_t cpu_get_c(const struct Cpu* cpu);
uint8_t cpu_get_d(const struct Cpu* cpu);
uint8_t cpu_get_e(const struct Cpu* cpu);
uint8_t cpu_get_h(const struct Cpu* cpu);
uint8_t cpu_get_l(const struct Cpu* cpu);
