#pragma once
#include <stdint.h>

struct Cpu {
  uint8_t A, B, C, D, E, F, H, L;
  uint16_t PC, SP;
  uint8_t halted;
  uint8_t ime;
  uint8_t ime_pending;
  uint8_t if_reg;
  uint8_t ie_reg;
  uint8_t cgb_mode;
  uint8_t ldbb_fired;  // Many tests exit by executing LD B, B
};

struct Bus;

struct Cpu* cpu_init(uint8_t cgb_mode);
void cpu_post_boot(struct Cpu* cpu, int cgb_mode, uint8_t header_checksum);
uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_check_interrupts(struct Cpu* cpu, struct Bus* bus);

uint8_t cpu_read_if(const struct Cpu* cpu);
void cpu_write_if(struct Cpu* cpu, uint8_t val);
uint8_t cpu_read_ie(const struct Cpu* cpu);
void cpu_write_ie(struct Cpu* cpu, uint8_t val);
