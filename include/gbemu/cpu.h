#pragma once
#include <stdint.h>

struct Bus;
struct Cpu;

struct Cpu* cpu_init(uint8_t cgb_mode);
void cpu_post_boot(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_read_if(const struct Cpu* cpu);
void cpu_write_if(struct Cpu* cpu, uint8_t val);
void cpu_req_intr(struct Cpu* cpu, uint8_t intr);
uint8_t cpu_read_ie(const struct Cpu* cpu);
void cpu_write_ie(struct Cpu* cpu, uint8_t val);
uint8_t cpu_get_cgb_mode(const struct Cpu* cpu);
uint8_t cpu_check_ld_b_b(struct Cpu* cpu);
uint8_t cpu_get_b(const struct Cpu* cpu);
uint8_t cpu_get_c(const struct Cpu* cpu);
uint8_t cpu_get_d(const struct Cpu* cpu);
uint8_t cpu_get_e(const struct Cpu* cpu);
uint8_t cpu_get_h(const struct Cpu* cpu);
uint8_t cpu_get_l(const struct Cpu* cpu);
