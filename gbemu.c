#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"
#include "cpu.h"
#include "dsp.h"

int main(int argc, char* argv[]) {
  // if(argc == 2) {
  init_reg();
  init_dsp();
  FILE* rom = fopen("bootrom.rom", "rb");
  FILE* tetr = fopen("tetris.gb", "rb");
  fread(mem, 0x8000, 1, tetr);
  fread(mem, 0x8000, 1, rom);
  while(PC < 0x100) if(exec(mem[PC ++])) break;
  printf("DONE\n");
  return 0;
  // }
}
