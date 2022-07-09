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
  int lines = 0;
  while(1) {
    int cyc = 456;
    while(cyc) cyc -= exec(mem[PC ++]);
    lines ++;
    scnln();
    if(lines == 154) {
      rndr();
      lines = 0;
    }
  }
  printf("DONE\n");
  return 0;
}
