#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/cpu_ops.h"

uint8_t cpu_step(struct Cpu* cpu, struct Bus* bus) {
  if (cpu->halted) return 4;
  if (cpu->ime_pending) {
    cpu->ime = true;
    cpu->ime_pending = false;
  }
  uint8_t op = bus_read(bus, cpu->PC++);
  if (op == 0xCB) {
    uint8_t prefix = bus_read(bus, cpu->PC++);
    return cpu_cb_ops[prefix](cpu, bus);
  }
  return cpu_ops[op](cpu, bus);
}
