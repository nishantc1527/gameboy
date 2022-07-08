#ifndef VAR
#define VAR

#include <string.h>

#define FLG_Z 7
#define FLG_N 6
#define FLG_H 5
#define FLG_C 4

#define CLR_WHT 0
#define CLR_L_GRY 1
#define CLR_D_GRY 2
#define CLR_BLK 3

typedef unsigned char byte;
typedef unsigned short dbyte;

byte mem[0x10000];
byte dsp[0xA0][0x90];
dbyte PC;
dbyte SP;
byte A, B, C, D, E, F, H, L;

void init_reg() {
  PC = 0;
  memset(mem, 0, sizeof(mem));
  memset(dsp, 0, sizeof(dsp));
  // mem[0xFF44] = 0x90;
}

#endif
