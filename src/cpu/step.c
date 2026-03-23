#include "cpu_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"

static uint8_t cb_bit_res_set(struct Cpu* cpu, struct Bus* bus, uint8_t op) {
  uint8_t group = op >> 6;
  uint8_t bit = (op >> 3) & 7;
  uint8_t r = op & 7;
  if (r == 6) {
    cpu_mem_tick(cpu, bus, 8);
    uint8_t val = bus_read(bus, gt_HL(cpu));
    if (group == 1) {
      c_bit(cpu, val, bit);
      return 12;
    }
    if (group == 2)
      val = (uint8_t)(val & ~(1u << bit));
    else
      set_bit(&val, bit);
    cpu_mem_tick(cpu, bus, 4);
    bus_write(bus, gt_HL(cpu), val);
    return 16;
  }
  uint8_t* reg;
  switch (r) {
    case 0:
      reg = &cpu->B;
      break;
    case 1:
      reg = &cpu->C;
      break;
    case 2:
      reg = &cpu->D;
      break;
    case 3:
      reg = &cpu->E;
      break;
    case 4:
      reg = &cpu->H;
      break;
    case 5:
      reg = &cpu->L;
      break;
    default:
      reg = &cpu->A;
      break;
  }
  if (group == 1) return (uint8_t)c_bit(cpu, *reg, bit);
  if (group == 2) return (uint8_t)c_res(reg, bit);
  return (uint8_t)c_set(reg, bit);
}

uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus) {
  if (cpu->halted) return 4;
  if (cpu->ime_pending) {
    cpu->ime = true;
    cpu->ime_pending = false;
  }
  uint8_t op = bus_read(bus, cpu->PC++);
  if (op == 0xCB) {
    uint8_t prefix = bus_read(bus, cpu->PC++);
    if (prefix >= 0x40) return cb_bit_res_set(cpu, bus, prefix);
    return cpu_cb_ops[prefix](cpu, bus);
  }
  return cpu_ops[op](cpu, bus);
}
