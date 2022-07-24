#include <stdio.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"
#include "cpu.h"
#include "dsp.h"

int update() {
  scn = 456;
  frame = 0;
  dma = 0;

  while(!frame) {
    int curr = exec(r_mem(PC)); 
    cnt ++;
    if(curr == -1) return 1;
    scn -= curr;
    if(scn <= 0) {
      scnln();
      scn = 456;
    }
    upd_stat();
    chck_intr();
    PC ++;
    memset(in, 0, sizeof(in));
    while(SDL_PollEvent(&evt)) {
      if(evt.type == SDL_QUIT) return 1;
      else if(evt.type == SDL_KEYDOWN) {
        switch(evt.key.keysym.sym) {
          case SDLK_s:
            in[BTN_A] = 1;
            break;
          case SDLK_a:
            in[BTN_B] = 1;
            break;
          case SDLK_RETURN:
            in[BTN_STRT] = 1;
            break;
          case SDLK_LSHIFT: case SDLK_RSHIFT:
            in[BTN_SLCT] = 1;
            break;
          case SDLK_UP:
            in[BTN_UP] = 1;
            break;
          case SDLK_DOWN:
            in[BTN_DOWN] = 1;
            break;
          case SDLK_LEFT:
            in[BTN_LEFT] = 1;
            break;
          case SDLK_RIGHT:
            in[BTN_RIGHT] = 1;
            break;
        }
      }
    }
    chck_in();
    chck_dma();
  }
  return 0;
}

int main(int argc, char* argv[]) {
  init_reg();
  init_dsp();
  FILE* b_rom = fopen("bootrom.rom", "rb");
  FILE* rom = fopen("dsptest.gb", "rb");
  fread(mem, 0x8000, 1, rom);
  fread(brom, 0x100, 1, b_rom);
  while(1) {
    if(update()) break;
    if(rndr()) break;
  }
  dbg();
  printf("DONE %d\n", cnt);
  return 0;
}
