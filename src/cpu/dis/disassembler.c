#include <stdint.h>
#include <stdio.h>

#include "../cpu_private.h"
#include "dis_private.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"

void disassemble(struct Cpu* cpu, struct Bus* bus, const uint16_t* watch_addrs,
                 uint8_t watch_count, uint64_t total_cycles) {
  if (!bus_read(bus, 0xFF50)) {
    return;
  }
  uint8_t instr = bus_read(bus, cpu->PC);
  uint16_t instr_pc = cpu->PC;
  printf("$%04X %02X ", instr_pc, instr);
  if (instr == 0xCB) {
    uint8_t prfx = bus_read(bus, (uint16_t)(cpu->PC + 1));
    printf("%02X ", prfx);
    dis_cb(cpu, bus, prfx);
  } else {
    if (instr <= 0x3F) {
      dis_00_3f(cpu, bus, instr);
    } else if (instr <= 0x7F) {
      dis_40_7f(cpu, bus, instr);
    } else if (instr <= 0xBF) {
      dis_80_bf(cpu, bus, instr);
    } else {
      dis_c0_ff(cpu, bus, instr);
    }
  }
  printf(
      "A:%d F:%d B:%d C:%d D:%d E:%d H:%d L:%d SP:%d PC:%d PCMEM:%d "
      "CYC:%llu\n",
      cpu->A, cpu->F, cpu->B, cpu->C, cpu->D, cpu->E, cpu->H, cpu->L, cpu->SP,
      cpu->PC, bus_read(bus, cpu->PC), (unsigned long long)total_cycles);
  for (uint8_t i = 0; i < watch_count; i++) {
    printf("MEM[%04X]=%02X\n", watch_addrs[i], bus_read(bus, watch_addrs[i]));
  }
}
