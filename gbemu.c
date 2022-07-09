#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"
#include "cpu.h"
#include "dsp.h"

int main(int argc, char* argv[]) {
  init_reg();
  init_dsp();
  FILE* rom = fopen("bootrom.rom", "rb");
  FILE* tetr = fopen("tetris.gb", "rb");
  fread(mem, 0x8000, 1, tetr);
  fread(mem, 0x8000, 1, rom);
  while(1) {
    int cyc = 456;
    while(cyc) exec(mem[PC ++]);
    scnln();
  }
  printf("DONE\n");
  return 0;
}
