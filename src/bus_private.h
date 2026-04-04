#pragma once

#include "gbemu/apu.h"
#include "gbemu/bus.h"
#include "gbemu/cpu.h"
#include "gbemu/dma.h"
#include "gbemu/joypad.h"
#include "gbemu/mmu.h"
#include "gbemu/ppu.h"
#include "gbemu/serial.h"
#include "gbemu/timer.h"

struct Bus {
  struct Mmu* mmu;
  struct Cpu* cpu;
  struct Ppu* ppu;
  struct Apu* apu;
  struct Timer* timer;
  struct Dma* dma;
  struct Joypad* joypad;
  struct Serial* serial;
};
