#include "gbemu/bus.h"

#include <stdlib.h>

#include "bus_private.h"

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
  cpu_req_intr(bus->cpu, intr);
}

void bus_notify_div_pulse(struct Bus* bus) { apu_notify_div_tick(bus->apu); }

static uint8_t bus_read_vram(struct Bus* bus, uint16_t addr) {
  if (ppu_blocks_vram(bus->ppu)) return BUS_OPEN_BUS;
  return mmu_read_vram(bus->mmu, addr);
}

static uint8_t bus_read_oam(struct Bus* bus, uint16_t addr) {
  if (ppu_blocks_oam(bus->ppu)) return BUS_OPEN_BUS;
  return mmu_read_oam(bus->mmu, (uint16_t)(addr - OAM_START));
}

static void bus_write_vram(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (ppu_blocks_vram(bus->ppu)) return;
  mmu_write_vram(bus->mmu, addr, val);
}

static void bus_write_oam(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (ppu_blocks_oam(bus->ppu)) return;
  mmu_write_oam(bus->mmu, (uint16_t)(addr - OAM_START), val);
}

static uint8_t io_read(struct Bus* bus, uint16_t addr) {
  if (addr == 0xFF00) return joypad_read(bus->joypad);
  if (addr == 0xFF01 || addr == 0xFF02) return serial_read(bus->serial, addr);
  if (addr >= 0xFF04 && addr <= 0xFF07) return timer_read(bus->timer, addr);
  if (addr == 0xFF0F) return cpu_read_if(bus->cpu);
  if (addr >= 0xFF10 && addr <= 0xFF3F) return apu_read(bus->apu, addr);
  if (addr >= 0xFF40 && addr <= 0xFF4B) return ppu_read(bus->ppu, addr);
  if (addr >= 0xFF51 && addr <= 0xFF55) return dma_hdma_read(bus->dma, addr);
  if (addr >= 0xFF68 && addr <= 0xFF6B) return ppu_read(bus->ppu, addr);
  if (addr == 0xFF4D) return mmu_read_cgb_speed(bus->mmu);
  if (addr == 0xFF4F) return mmu_get_vram_bank(bus->mmu);
  if (addr == 0xFF70) return mmu_get_wram_bank(bus->mmu);
  return BUS_OPEN_BUS;
}

static void io_write(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (addr == 0xFF00) {
    joypad_write(bus->joypad, val);
    return;
  }
  if (addr == 0xFF01 || addr == 0xFF02) {
    serial_write(bus->serial, addr, val, bus);
    return;
  }
  if (addr >= 0xFF04 && addr <= 0xFF07) {
    timer_write(bus->timer, addr, val, bus);
    return;
  }
  if (addr == 0xFF0F) {
    cpu_write_if(bus->cpu, val);
    return;
  }
  if (addr >= 0xFF10 && addr <= 0xFF3F) {
    apu_write(bus->apu, addr, val);
    return;
  }
  if (addr == 0xFF46) {
    dma_trigger(bus->dma, val);
    return;
  }
  if (addr >= 0xFF51 && addr <= 0xFF55) {
    dma_hdma_write(bus->dma, bus, addr, val);
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
  if (addr == 0xFF4D) {
    mmu_write_cgb_speed(bus->mmu, val);
    return;
  }
  if (addr == 0xFF4F) {
    mmu_set_vram_bank(bus->mmu, val);
    return;
  }
  if (addr == 0xFF50) {
    mmu_disable_boot(bus->mmu);
    return;
  }
  if (addr == 0xFF70) {
    mmu_set_wram_bank(bus->mmu, val);
    return;
  }
}

uint8_t bus_read(struct Bus* bus, uint16_t addr) {
  if (dma_blocks_cpu(bus->dma) && addr < HRAM_START) return BUS_OPEN_BUS;
  if (mmu_boot_active(bus->mmu)) {
    if (addr < 0x0100) return mmu_read_boot(bus->mmu, addr);
    if (mmu_is_cgb(bus->mmu) && addr >= 0x0200 && addr < 0x0900)
      return mmu_read_boot(bus->mmu, addr);
  }
  if (addr < VRAM_START) return mmu_read_rom(bus->mmu, addr);
  if (addr < ERAM_START) return bus_read_vram(bus, addr);
  if (addr < WRAM_START) return mmu_read_eram(bus->mmu, addr);
  if (addr < ECHO_START) return mmu_read_wram(bus->mmu, addr);
  if (addr < OAM_START) return bus_read(bus, (uint16_t)(addr - ECHO_OFFSET));
  if (addr < OAM_END + 1) return bus_read_oam(bus, addr);
  if (addr < IO_START) return BUS_OPEN_BUS;
  if (addr < HRAM_START) return io_read(bus, addr);
  if (addr == IE_REG_ADDR) return cpu_read_ie(bus->cpu);
  return mmu_read_hram(bus->mmu, (uint16_t)(addr - HRAM_START));
}

void bus_write(struct Bus* bus, uint16_t addr, uint8_t val) {
  if (dma_blocks_cpu(bus->dma) && addr < HRAM_START) return;
  if (addr < VRAM_START) {
    mmu_write_rom(bus->mmu, addr, val);
    return;
  }
  if (addr < ERAM_START) {
    bus_write_vram(bus, addr, val);
    return;
  }
  if (addr < WRAM_START) {
    mmu_write_eram(bus->mmu, addr, val);
    return;
  }
  if (addr < ECHO_START) {
    mmu_write_wram(bus->mmu, addr, val);
    return;
  }
  if (addr < OAM_START) {
    bus_write(bus, (uint16_t)(addr - ECHO_OFFSET), val);
    return;
  }
  if (addr < OAM_END + 1) {
    bus_write_oam(bus, addr, val);
    return;
  }
  if (addr < IO_START) return;
  if (addr < HRAM_START) {
    io_write(bus, addr, val);
    return;
  }
  if (addr == IE_REG_ADDR) {
    cpu_write_ie(bus->cpu, val);
    return;
  }
  mmu_write_hram(bus->mmu, (uint16_t)(addr - HRAM_START), val);
}
