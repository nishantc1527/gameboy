#include "gbemu/bus.h"

#include <stdlib.h>

#include "gbemu/apu.h"
#include "gbemu/cpu.h"
#include "gbemu/dma.h"
#include "gbemu/joypad.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/serial.h"
#include "gbemu/timer.h"

struct Bus* bus_init(struct Mmu* mmu, struct Cpu* cpu, struct Ppu* ppu,
                     struct Apu* apu, struct Timer* timer, struct Dma* dma,
                     struct Joypad* joypad, struct Serial* serial) {
  struct Bus* b = malloc(sizeof(struct Bus));
  b->mmu = mmu;
  b->cpu = cpu;
  b->ppu = ppu;
  b->apu = apu;
  b->timer = timer;
  b->dma = dma;
  b->joypad = joypad;
  b->serial = serial;
  return b;
}

void bus_free(struct Bus* b) { free(b); }

void bus_req_intr(struct Bus* bus, uint8_t intr) {
  bus->cpu->if_reg |= (uint8_t)(1u << intr);
}

static uint8_t io_read(struct Bus* bus, uint16_t addr) {
  if (addr == 0xFF00) return joypad_read(bus->joypad);
  if (addr == 0xFF01 || addr == 0xFF02) return serial_read(bus->serial, addr);
  if (addr >= 0xFF04 && addr <= 0xFF07) return timer_read(bus->timer, addr);
  if (addr == 0xFF0F) return bus->cpu->if_reg | 0xE0;
  if (addr >= 0xFF10 && addr <= 0xFF3F) return apu_read(bus->apu, addr);
  if (addr >= 0xFF40 && addr <= 0xFF4B) return ppu_read(bus->ppu, addr);
  if (addr >= 0xFF68 && addr <= 0xFF6B) return ppu_read(bus->ppu, addr);
  return mmu_r_mem(bus->mmu, addr);
}

static void io_write(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (addr == 0xFF00) {
    joypad_write(bus->joypad, val);
    return;
  }
  if (addr == 0xFF01 || addr == 0xFF02) {
    serial_write(bus->serial, addr, val, bus->cpu);
    return;
  }
  if (addr >= 0xFF04 && addr <= 0xFF07) {
    timer_write(bus->timer, addr, val, bus->apu, bus->cpu);
    return;
  }
  if (addr == 0xFF0F) {
    bus->cpu->if_reg = val & 0x1Fu;
    return;
  }
  if (addr >= 0xFF10 && addr <= 0xFF3F) {
    apu_write(bus->apu, addr, val);
    return;
  }
  if (addr == 0xFF46) {
    dma_trigger(bus->dma, bus, val);
    return;
  }
  if (addr >= 0xFF40 && addr <= 0xFF4B) {
    ppu_write(bus->ppu, addr, val);
    return;
  }
  if (addr >= 0xFF68 && addr <= 0xFF6B) {
    ppu_write(bus->ppu, addr, val);
    return;
  }
  mmu_w_mem(bus->mmu, addr, val);
}

uint8_t bus_read(struct Bus* bus, uint16_t addr) {
  if (mmu_boot_active(bus->mmu)) {
    if (addr < 0x0100) return mmu_read_boot(bus->mmu, addr);
    if (mmu_is_cgb(bus->mmu) && addr >= 0x0200 && addr < 0x0900)
      return mmu_read_boot(bus->mmu, addr);
  }
  if (addr < 0x8000) return mmu_read_rom(bus->mmu, addr);
  if (addr < 0xA000) {
    uint8_t mode = bus->ppu->stat & 0x03;
    if ((bus->ppu->lcdc & 0x80) && mode == 3) return 0xFF;
    return mmu_read_vram(bus->mmu, addr);
  }
  if (addr < 0xC000) return mmu_read_eram(bus->mmu, addr);
  if (addr < 0xE000) return mmu_read_wram(bus->mmu, addr);
  if (addr < 0xFE00) return bus_read(bus, (uint16_t)(addr - 0x2000));
  if (addr < 0xFEA0) {
    uint8_t mode = bus->ppu->stat & 0x03;
    if ((bus->ppu->lcdc & 0x80) && (mode == 2 || mode == 3)) return 0xFF;
    return mmu_read_oam(bus->mmu, (uint16_t)(addr - 0xFE00));
  }
  if (addr < 0xFF00) return 0xFF;
  if (addr < 0xFF80) return io_read(bus, addr);
  if (addr == 0xFFFF) return bus->cpu->ie_reg;
  return mmu_read_hram(bus->mmu, (uint16_t)(addr - 0xFF80u));
}

void bus_write(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (addr < 0x8000) {
    mmu_write_rom(bus->mmu, addr, val);
    return;
  }
  if (addr < 0xA000) {
    uint8_t mode = bus->ppu->stat & 0x03;
    if ((bus->ppu->lcdc & 0x80) && mode == 3) return;
    mmu_write_vram(bus->mmu, addr, val);
    return;
  }
  if (addr < 0xC000) {
    mmu_write_eram(bus->mmu, addr, val);
    return;
  }
  if (addr < 0xE000) {
    mmu_write_wram(bus->mmu, addr, val);
    return;
  }
  if (addr < 0xFE00) {
    bus_write(bus, (uint16_t)(addr - 0x2000), val);
    return;
  }
  if (addr < 0xFEA0) {
    uint8_t mode = bus->ppu->stat & 0x03;
    if ((bus->ppu->lcdc & 0x80u) && (mode == 2 || mode == 3)) return;
    mmu_write_oam(bus->mmu, (uint16_t)(addr - 0xFE00u), val);
    return;
  }
  if (addr < 0xFF00) return;
  if (addr < 0xFF80) {
    io_write(bus, addr, val);
    return;
  }
  if (addr == 0xFFFF) {
    bus->cpu->ie_reg = val;
    return;
  }
  mmu_write_hram(bus->mmu, (uint16_t)(addr - 0xFF80u), val);
}
