#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"
#include "cpu.h"
#include "dsp.h"

int cnt = 0;

int update() {
  int cyc = 70224;
  int scn = 456;

  while(cyc > 0) {
    int curr = exec(rd8()); 
    cnt ++;
    if(curr == -1) return 1;
    scn -= curr;
    cyc -= curr;
    if(scn <= 0) {
      scnln();
      scn = 456;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
  init_reg();
  init_dsp();
  FILE* b_rom = fopen("bootrom.rom", "rb");
  FILE* rom = fopen("tetris.gb", "rb");
  fread(mem, 0x8000, 1, rom);
  fread(brom, 0x100, 1, b_rom);
  while(1) {
    if(update()) break;
    if(rndr()) break;
  }
  printf("DONE %d\n", cnt);
  return 0;
}
