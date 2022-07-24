#ifndef VAR
#define VAR

#include <string.h>

#include <SDL2/SDL.h>

#define FLG_Z      7
#define FLG_N      6
#define FLG_H      5
#define FLG_C      4

#define CLR_WHT    0
#define CLR_L_GRY  1
#define CLR_D_GRY  2
#define CLR_BLK    3

#define HEX_WHT    0xFF
#define HEX_L_GREY 0xD3
#define HEX_R_GREY 0x69
#define HEX_BLK    0x00

#define JOYP       r_mem(0xFF00)
#define IF         r_mem(0xFF0F)
#define LCDC       r_mem(0xFF40)
#define LCD_STAT   r_mem(0xFF41)
#define SCY        r_mem(0xFF42)
#define SCX        r_mem(0xFF43)
#define LY         r_mem(0xFF44)
#define LYC        r_mem(0xFF45)
#define DMA        r_mem(0xFF46)
#define BGP        r_mem(0xFF47)
#define OBP0       r_mem(0xFF48)
#define OBP1       r_mem(0xFF49)
#define IE         r_mem(0xFFFF)

#define BTN_A      0
#define BTN_B      1
#define BTN_STRT   2
#define BTN_SLCT   3
#define BTN_UP     4
#define BTN_DOWN   5
#define BTN_LEFT   6
#define BTN_RIGHT  7

typedef unsigned char byte;
typedef unsigned short dbyte;

byte mem[0x10000];
byte brom[0x100];
byte dsp[0x90][0xA0];
dbyte PC;
dbyte SP;
byte A, B, C, D, E, F, H, L;
byte IME;
int scn, frame;
int in[8];
int dma;

SDL_Window* win;
SDL_Window* tile_dat;

SDL_Renderer* rnd;
SDL_Renderer* rnd_dat;

SDL_Event evt;

int FCT = 5;
int FCT_DAT = 3;
dbyte STP = 0xFFFF;
int cnt = 0;

void init_reg() {
  PC = 0;
  memset(mem, 0, sizeof(mem));
  memset(dsp, 0, sizeof(dsp));
}

#endif
