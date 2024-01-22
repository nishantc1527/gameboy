#include <stdio.h>
#include <string.h>
#include <SDL.h>

#define BTN_A       0
#define BTN_B       1
#define BTN_START   2
#define BTN_SELECT  3
#define BTN_UP      4
#define BTN_DOWN    5
#define BTN_LEFT    6
#define BTN_RIGHT   7

#define CLR_WHT     0
#define CLR_L_GRY   1
#define CLR_D_GRY   2
#define CLR_BLK     3
#define CLR_EXT     4

#define CPU_FREQ    4194304
#define DIV_FREQ    16384
#define TIM_FREQ_1  4096
#define TIM_FREQ_2  262144
#define TIM_FREQ_3  65536
#define TIM_FREQ_4  16384

#define FLG_Z       7
#define FLG_N       6
#define FLG_H       5
#define FLG_C       4

#define HEX_WHT     0xeeeeee
#define HEX_L_GREY  0xf26161
#define HEX_R_GREY  0xea0909
#define HEX_BLK     0x9a0707
#define HEX_EXT     0xFF0000

#define INTR_VBLANK 0
#define INTR_LCD    1
#define INTR_TIMER  2
#define INTR_SERIAL 3
#define INTR_JOYPAD 4

#define SCRN_WIDTH  0xA0
#define SCRN_HEIGHT 0x90

#define JOYP        r_mem(0xFF00)
#define SB	        r_mem(0xFF01)
#define SC          r_mem(0xFF02)
#define DIV         r_mem(0xFF04)
#define TIMA        r_mem(0xFF05)
#define TMA         r_mem(0xFF06)
#define TAC         r_mem(0xFF07)
#define IF          r_mem(0xFF0F)
#define LCDC        r_mem(0xFF40)
#define LCD_STAT    r_mem(0xFF41)
#define SCY         r_mem(0xFF42)
#define SCX         r_mem(0xFF43)
#define LY          r_mem(0xFF44)
#define LYC         r_mem(0xFF45)
#define DMA         r_mem(0xFF46)
#define BGP         r_mem(0xFF47)
#define OBP0        r_mem(0xFF48)
#define OBP1        r_mem(0xFF49)
#define WY          r_mem(0xFF4A)
#define WX          r_mem(0xFF4B)
#define IE          r_mem(0xFFFF)

typedef unsigned char byte;
typedef unsigned short dbyte;

byte mem[0x10000];
byte brom[0x100];
byte rom[0x800000];
byte extern_ram[0x20000];
byte dsp[SCRN_HEIGHT][SCRN_WIDTH];
int upd[SCRN_HEIGHT][SCRN_WIDTH];
dbyte PC;
dbyte SP;
byte A, B, C, D, E, F, H, L;
byte IME, WIN_CNT, HALT;
int scn, frame, tim_cnt, tim_thresh, div_cnt;
int in[8];
dbyte intr_loc[] = { 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };

char title[20];
byte cart_type;
byte rom_size;
unsigned int rom_size_bytes;
byte ram_size;
byte rom_bank;
byte ram_bank;
int ram_enable;

SDL_Window* win;
SDL_Renderer* rnd;
SDL_Event evt;

char* rom_name = "tetris.gb";
int FCT_X = 6, FCT_Y = 6;
int dis = 0;
int dbg_time = 5000;

void redraw();

void init_reg() {
    PC = 0;
    WIN_CNT = 0;
    HALT = 0;
    memset(mem, 0, sizeof(mem));
    memset(dsp, 0, sizeof(dsp));
    memset(rom, 0, sizeof(rom));
    memset(extern_ram, 0, sizeof(extern_ram));
    redraw();
    div_cnt = 0;
    tim_cnt = 0;
}

int init_dsp() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf("ERROR CREATING WINDOW: %s\n", SDL_GetError());
        return 1;
    }
    win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCRN_WIDTH * FCT_X, SCRN_HEIGHT * FCT_Y, SDL_WINDOW_SHOWN);
    rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    return 0;
}

int cart_info() {
    memset(title, 0, sizeof(title));
    dbyte i = 0x0134;
    for (; i <= 0x0142; i++) {
        if (!mem[i]) break;
        title[i - 0x0134] = mem[i];
    }
    cart_type = mem[0x0147];
    rom_size = mem[0x0148];
    ram_size = mem[0x0149];
    switch (cart_type) {
    case 0x00:
        break;
    default:
        printf("UNIMPLEMENTED MAPPER $%02X\n", cart_type);
        return 1;
    }
    switch (rom_size) {
    case 0x00:
        rom_size_bytes = 32;
        break;
    case 0x05:
        rom_size_bytes = 1024;
        break;
    default:
        printf("UNIMPLEMENTED ROM SIZE $%02X\n", rom_size);
        return 1;
    }
    rom_size_bytes *= 0x400;
    switch (ram_size) {
    case 0x00:
    case 0x03:
        break;
    default:
        printf("UNIMPLEMENTED RAM SIZE $%02X\n", ram_size);
        return 1;
    }
    rom_bank = 1;
    ram_enable = 0;
    return 0;
}

byte r_mem(dbyte loc) {
    if (!mem[0xFF50] && loc < 0x100) return brom[loc];
    if (loc < 0x8000) {
        switch (cart_type) {
        case 0x00:
            return rom[loc];
        }
    }
    if (loc >= 0xA000 && loc < 0xC000) {
        switch (cart_type) {
        case 0x00:
            return 0xFF;
        }
    }
    if (loc >= 0xE000 && loc <= 0xFDFF) loc -= 0x200;
    return mem[loc];
}

void w_mem(dbyte loc, byte val) {
    if (loc < 0x8000) {
        switch (cart_type) {
        case 0x00:
            return;
        }
    }
    if (loc >= 0xA000 && loc < 0xC000) {
        switch (cart_type) {
        case 0x00:
            return;
        }
    }
    if (loc >= 0xE000 && loc <= 0xFDFF) loc -= 0x200;
    if (loc == 0xFF04) val = 0;
    if (loc == 0xFF07 && (val >> 2) & 1) {
        switch (val & 0b11) {
        case 0b00:
            tim_thresh = TIM_FREQ_1;
            break;
        case 0b01:
            tim_thresh = TIM_FREQ_2;
            break;
        case 0b10:
            tim_thresh = TIM_FREQ_3;
            break;
        case 0b11:
            tim_thresh = TIM_FREQ_4;
            break;
        }
        tim_thresh = CPU_FREQ / tim_thresh;
    }
    mem[loc] = val;
}

void w_pxl(int y, int x, int clr) {
    if (dsp[y][x] != clr) upd[y][x] = 1;
    dsp[y][x] = clr;
}

byte rd8() {
    return r_mem(++PC);
}

dbyte rd16() {
    dbyte addr1 = ++PC;
    dbyte addr2 = ++PC;
    return ((dbyte)r_mem(addr2) << 8) | (dbyte)r_mem(addr1);
}

void psh16(dbyte val) {
    byte val1 = (byte)(val >> 8);
    byte val2 = (byte)val;
    w_mem(SP - 1, val1);
    w_mem(SP - 2, val2);
    SP -= 2;
}

dbyte pop16() {
    dbyte val1 = r_mem(SP);
    dbyte val2 = r_mem(SP + 1);
    SP += 2;
    return val1 | (val2 << 8);
}

dbyte pk16() {
    dbyte val = pop16();
    psh16(val);
    return val;
}

dbyte gt_AF() {
    return (((dbyte)A) << 8) | (dbyte)F;
}

void st_AF(dbyte AF) {
    A = (byte)(AF >> 8);
    F = (byte)AF;
}

dbyte gt_BC() {
    return (((dbyte)B) << 8) | (dbyte)C;
}

void st_BC(dbyte BC) {
    B = (byte)(BC >> 8);
    C = (byte)BC;
}

dbyte gt_DE() {
    return (((dbyte)D) << 8) | (dbyte)E;
}

void st_DE(dbyte DE) {
    D = (byte)(DE >> 8);
    E = (byte)DE;
}

dbyte gt_HL() {
    return (((dbyte)H) << 8) | (dbyte)L;
}

void st_HL(dbyte HL) {
    H = (byte)(HL >> 8);
    L = (byte)HL;
}

int gt_clr(byte pal, int val) {
    return (pal >> (val << 1)) & 0b11;
}

byte gt_bt(byte var, byte bt) {
    return (var >> bt) & 1;
}

void st_bt(byte* var, byte bt) {
    *var |= (1 << bt);
}

void cl_bt(byte* var, byte bt) {
    *var &= ~(1 << bt);
}

byte gt_flg(byte flg) {
    return gt_bt(F, flg);
}

void st_flg(byte flg) {
    st_bt(&F, flg);
}

void cl_flg(byte flg) {
    cl_bt(&F, flg);
}

void st_z(byte var) {
    if (var == 0) st_flg(FLG_Z);
    else cl_flg(FLG_Z);
}

void st_h_add(byte var1, byte var2) {
    if (((var1 & 0xF) + (var2 & 0xF)) & 0x10) st_flg(FLG_H);
    else cl_flg(FLG_H);
}

void st_h_add16(dbyte var1, dbyte var2) {
    if (((var1 & 0xFFF) + (var2 & 0xFFF)) & 0x1000) st_flg(FLG_H);
    else cl_flg(FLG_H);
}

void st_h_sub(byte var1, byte var2) {
    if (((var1 & 0x0F) - (var2 & 0x0F)) & 0x10) st_flg(FLG_H);
    else cl_flg(FLG_H);
}

void st_c_rl(byte var) {
    if (var >> 7) st_flg(FLG_C);
    else cl_flg(FLG_C);
}

void st_c_rr(byte var) {
    if (var & 1) st_flg(FLG_C);
    else cl_flg(FLG_C);
}

void st_c_add(byte var1, byte var2) {
    dbyte res = (dbyte)var1 + (dbyte)var2;
    if (res > 0xFF) st_flg(FLG_C);
    else cl_flg(FLG_C);
}

void st_c_add16(dbyte var1, dbyte var2) {
    int res = (int)var1 + (int)var2;
    if (res > 0xFFFF) st_flg(FLG_C);
    else cl_flg(FLG_C);
}

void st_c_sub(byte var1, byte var2) {
    if (var1 < var2) st_flg(FLG_C);
    else cl_flg(FLG_C);
}

void kp() {
    PC--;
}

int c_add(byte reg);

int c_adc(byte reg) {
    int cy = 0;
    int hy = 0;
    if (gt_flg(FLG_C)) {
        st_h_add(A, 1);
        st_c_add(A, 1);
        cy = gt_flg(FLG_C);
        hy = gt_flg(FLG_H);
        A++;
    }
    c_add(reg);
    if (cy) st_flg(FLG_C);
    if (hy) st_flg(FLG_H);
    return 4;
}

int c_add(byte reg) {
    st_h_add(A, reg);
    st_c_add(A, reg);
    A += reg;
    st_z(A);
    cl_flg(FLG_N);
    return 4;
}

int c_and(byte reg) {
    A &= reg;
    st_z(A);
    cl_flg(FLG_N);
    st_flg(FLG_H);
    cl_flg(FLG_C);
    return 4;
}

int c_bit(byte reg, int bit) {
    if (gt_bt(reg, bit)) cl_flg(FLG_Z);
    else st_flg(FLG_Z);
    cl_flg(FLG_N);
    st_flg(FLG_H);
    return 8;
}

int c_call(int flg) {
    if (flg) {
        psh16(PC + 3);
        PC = rd16();
        kp();
        return 24;
    }
    rd16();
    return 12;
}

int c_cp(byte reg) {
    st_z(A - reg);
    st_flg(FLG_N);
    st_h_sub(A, reg);
    st_c_sub(A, reg);
    return 4;
}

int c_cpl(byte* reg) {
    *reg = ~*reg;
    st_flg(FLG_N);
    st_flg(FLG_H);
    return 4;
}

int c_dec(byte* reg) {
    st_h_sub(*reg, 1);
    *reg = *reg - 1;
    st_z(*reg);
    st_flg(FLG_N);
    return 4;
}

int c_dec_mem(dbyte loc) {
    byte reg = r_mem(loc);
    st_h_sub(reg, 1);
    reg = reg - 1;
    w_mem(loc, reg);
    st_z(reg);
    st_flg(FLG_N);
    return 12;
}

int c_inc(byte* reg) {
    st_h_add(*reg, 1);
    *reg = *reg + 1;
    st_z(*reg);
    cl_flg(FLG_N);
    return 4;
}

int c_inc_mem(dbyte loc) {
    byte reg = r_mem(loc);
    st_h_add(reg, 1);
    reg = reg + 1;
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    return 12;
}

int c_jp8(int flg) {
    if (flg) {
        PC += (signed char)rd8();
        return 12;
    }
    else {
        rd8();
        return 8;
    }
}

int c_jp16(int flg) {
    if (flg) {
        PC = rd16();
        kp();
        return 16;
    }
    else rd16();
    return 12;
}

int c_or(byte reg) {
    A |= reg;
    st_z(A);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    cl_flg(FLG_C);
    return 4;
}

int c_res(byte* reg, int bit) {
    *reg = *reg & ~(1 << bit);
    return 8;
}

int c_res_mem(dbyte loc, int bit) {
    byte reg = r_mem(loc);
    reg = reg & ~(1 << bit);
    w_mem(loc, reg);
    return 16;
}

int c_ret(int flg) {
    if (flg) {
        PC = pop16();
        kp();
        return 20;
    }
    else {
        return 8;
    }
}

int c_rr(byte* reg) {
    int carry = gt_flg(FLG_C);
    st_c_rr(*reg);
    *reg = (*reg >> 1) | (carry << 7);
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_rr_mem(dbyte loc) {
    byte reg = r_mem(loc);
    int carry = gt_flg(FLG_C);
    st_c_rr(reg);
    reg = (reg >> 1) | (carry << 7);
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_rrc(byte* reg) {
    int carry = *reg & 1;
    st_c_rr(*reg);
    *reg = (*reg >> 1) | (carry << 7);
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_rrc_mem(dbyte loc) {
    byte reg = r_mem(loc);
    int carry = reg & 1;
    st_c_rr(reg);
    reg = (reg >> 1) | (carry << 7);
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_rl(byte* reg) {
    int carry = gt_flg(FLG_C);
    st_c_rl(*reg);
    *reg = (*reg << 1) | carry;
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_rl_mem(dbyte loc) {
    byte reg = r_mem(loc);
    int carry = gt_flg(FLG_C);
    st_c_rl(reg);
    reg = (reg << 1) | carry;
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_rlc(byte* reg) {
    int carry = (*reg >> 7) & 1;
    st_c_rl(*reg);
    *reg = (*reg << 1) | carry;
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_rlc_mem(dbyte loc) {
    byte reg = r_mem(loc);
    int carry = (reg >> 7) & 1;
    st_c_rl(reg);
    reg = (reg << 1) | carry;
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_rst(byte loc) {
    psh16(PC + 1);
    PC = loc;
    kp();
    return 16;
}

int c_sub(byte reg);

int c_sbc(byte reg) {
    int cy = 0;
    int hy = 0;
    if (gt_flg(FLG_C)) {
        st_h_sub(A, 1);
        st_c_sub(A, 1);
        cy = gt_flg(FLG_C);
        hy = gt_flg(FLG_H);
        A--;
    }
    c_sub(reg);
    if (cy) st_flg(FLG_C);
    if (hy) st_flg(FLG_H);
    return 4;
}

int c_srl(byte* reg) {
    if (*reg & 1) st_flg(FLG_C);
    else cl_flg(FLG_C);
    *reg = *reg >> 1;
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_srl_mem(dbyte loc) {
    byte reg = r_mem(loc);
    if (reg & 1) st_flg(FLG_C);
    else cl_flg(FLG_C);
    reg = reg >> 1;
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_set(byte* reg, int bit) {
    st_bt(reg, bit);
    return 8;
}

int c_set_mem(dbyte loc, int bit) {
    byte reg = r_mem(loc);
    st_bt(&reg, bit);
    w_mem(loc, reg);
    return 16;
}

int c_sla(byte* reg) {
    st_c_rl(*reg);
    *reg = *reg << 1;
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_sla_mem(dbyte loc) {
    byte reg = r_mem(loc);
    st_c_rl(reg);
    reg = reg << 1;
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_sra(byte* reg) {
    st_c_rr(*reg);
    int bt = (*reg >> 7) & 1;
    *reg = (*reg >> 1) | (bt << 7);
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 8;
}

int c_sra_mem(dbyte loc) {
    byte reg = r_mem(loc);
    st_c_rr(reg);
    int bt = (reg >> 7) & 1;
    reg = (reg >> 1) | (bt << 7);
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    return 16;
}

int c_sub(byte reg) {
    st_h_sub(A, reg);
    st_c_sub(A, reg);
    A -= reg;
    st_z(A);
    st_flg(FLG_N);
    return 4;
}

int c_swp(byte* reg) {
    *reg = (*reg >> 4) | (*reg << 4);
    st_z(*reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    cl_flg(FLG_C);
    return 8;
}

int c_swp_mem(dbyte loc) {
    byte reg = r_mem(loc);
    reg = (reg >> 4) | (reg << 4);
    w_mem(loc, reg);
    st_z(reg);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    cl_flg(FLG_C);
    return 16;
}

int c_xor(int reg) {
    A ^= reg;
    st_z(A);
    cl_flg(FLG_N);
    cl_flg(FLG_H);
    cl_flg(FLG_C);
    return 4;
}

void dbg() {
    printf("{\n\tAF: $%04X\n\tBC: $%04X\n\tDE: $%04X\n\tHL: $%04X\n\tSP: $%04X\n\tPC: $%04X\n\tSTACK PEEK: $%04X\n\tZERO FLAG: %d\n\tSUBTRACTION FLAG: %d\n\tHALF CARRY FLAG: %d\n\tCARRY FLAG: %d\n}\n", gt_AF(), gt_BC(), gt_DE(), gt_HL(), SP, PC, pk16(), gt_flg(FLG_Z), gt_flg(FLG_N), gt_flg(FLG_H), gt_flg(FLG_C));
}

void req_intr(int intr) {
    st_bt(&mem[0xFF0F], intr);
}

void do_intr(int intr) {
    if (IME) {
        switch (-1) { // intr
        case INTR_VBLANK:
            printf("VBLANK INTERRUPT\n");
            break;
        case INTR_LCD:
            printf("LCD STAT INTERRUPT\n");
            break;
        case INTR_TIMER:
            printf("TIMER INTERRUPT\n");
            break;
        case INTR_SERIAL:
            printf("SERIAL INTERRUPT\n");
            break;
        case INTR_JOYPAD:
            printf("JOYPAD INTERRUPT\n");
            break;
        }
        cl_bt(&mem[0xFF0F], intr);
        psh16(PC);
        PC = intr_loc[intr];
    }
    HALT = 0;
    IME = 0;
}

void chck_intr() {
    for (int intr = 0; intr < 5; intr++) {
        if (gt_bt(IF, intr) && gt_bt(IE, intr)) {
            do_intr(intr);
        }
    }
}

void intr_vblank_lcd(byte stat, int prev_mode, int curr_mode) {
    int req_vblank = 0;
    int req_lcd = 0;
    if (prev_mode != curr_mode) {
        if (curr_mode == 1) req_vblank = 1;
        if (curr_mode == 0 && gt_bt(stat, 3)) req_lcd = 1;
        if (curr_mode == 1 && gt_bt(stat, 4)) req_lcd = 1;
        if (curr_mode == 2 && gt_bt(stat, 5)) req_lcd = 1;
    }
    int prev_lyc = gt_bt(stat, 2);
    int curr_lyc = LY == LYC;
    if (prev_lyc != curr_lyc && curr_lyc && gt_bt(stat, 6)) req_lcd = 1;
    if (req_vblank) req_intr(INTR_VBLANK);
    if (req_lcd) req_intr(INTR_LCD);
}

void intr_timer(byte tima) {
    if (tima == 0xFF) req_intr(INTR_TIMER);
}

void intr_serial() {
    // TODO
}

void intr_joypad(byte prev_joyp, byte curr_joyp) {
    int req = 0;
    for (int i = 0; i < 4; i++) {
        int prev = (prev_joyp >> i) & 1;
        int curr = (curr_joyp >> i) & 1;
        if (prev != curr && curr == 0) req = 1;
    }
    if (req) req_intr(INTR_JOYPAD);
}

void upd_lcd() {
    byte stat = LCD_STAT;
    int prev_mode = stat & 0b11;
    int curr_mode;
    if (scn >= 456 - 80) curr_mode = 2;
    else if (scn >= 456 - 80 - 168) curr_mode = 3;
    else curr_mode = 0;
    if (LY >= SCRN_HEIGHT) curr_mode = 1;
    intr_vblank_lcd(stat, prev_mode, curr_mode);
    stat &= ~(0b11);
    stat |= curr_mode;
    // printf("Mode: $%02X\nLY: $%02X\nLCDC: $%02X\n", curr_mode, LY, LCDC);
    if (LY == LYC) st_bt(&stat, 2);
    else cl_bt(&stat, 2);
    w_mem(0xFF41, stat);
}

void upd_tim(int cycles) {
    div_cnt += cycles;
    while (div_cnt >= CPU_FREQ / DIV_FREQ) {
        byte div = DIV;
        div++;
        mem[0xFF04] = div;
        div_cnt -= CPU_FREQ / DIV_FREQ;
    }
    if (gt_bt(TAC, 2)) {
        tim_cnt += cycles;
        while (tim_cnt >= tim_thresh) {
            byte tima = TIMA;
            intr_timer(tima);
            if (tima == 0xFF) tima = TMA;
            else tima++;
            w_mem(0xFF05, tima);
            tim_cnt -= tim_thresh;
        }
    }
}

int upd_in() {
    byte curr_joyp = JOYP;
    curr_joyp |= 0xF;
    if (!gt_bt(curr_joyp, 4)) {
        if (in[BTN_RIGHT]) cl_bt(&curr_joyp, 0);
        if (in[BTN_LEFT]) cl_bt(&curr_joyp, 1);
        if (in[BTN_UP]) cl_bt(&curr_joyp, 2);
        if (in[BTN_DOWN]) cl_bt(&curr_joyp, 3);
    }
    if (!gt_bt(curr_joyp, 5)) {
        if (in[BTN_A]) cl_bt(&curr_joyp, 0);
        if (in[BTN_B]) cl_bt(&curr_joyp, 1);
        if (in[BTN_SELECT]) cl_bt(&curr_joyp, 2);
        if (in[BTN_START]) cl_bt(&curr_joyp, 3);
    }
    intr_joypad(JOYP, curr_joyp);
    w_mem(0xFF00, curr_joyp);
    return 0;
}

int handle_in() {
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
        case SDL_QUIT: return 1;
        case SDL_KEYDOWN:
            switch (evt.key.keysym.sym) {
            case SDLK_s:
                in[BTN_A] = 1;
                break;
            case SDLK_a:
                in[BTN_B] = 1;
                break;
            case SDLK_RETURN:
                in[BTN_START] = 1;
                break;
            case SDLK_LSHIFT: case SDLK_RSHIFT:
                in[BTN_SELECT] = 1;
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
            break;
        case SDL_KEYUP:
            switch (evt.key.keysym.sym) {
            case SDLK_s:
                in[BTN_A] = 0;
                break;
            case SDLK_a:
                in[BTN_B] = 0;
                break;
            case SDLK_RETURN:
                in[BTN_START] = 0;
                break;
            case SDLK_LSHIFT: case SDLK_RSHIFT:
                in[BTN_SELECT] = 0;
                break;
            case SDLK_UP:
                in[BTN_UP] = 0;
                break;
            case SDLK_DOWN:
                in[BTN_DOWN] = 0;
                break;
            case SDLK_LEFT:
                in[BTN_LEFT] = 0;
                break;
            case SDLK_RIGHT:
                in[BTN_RIGHT] = 0;
                break;
            }
            break;
        }
    }
    upd_in();
}

void upd_dma() {
    if (DMA >= 0x00 && DMA <= 0xDF) {
        dbyte src = DMA * 0x100;
        for (dbyte t = 0; t < 0xA0; t++) {
            w_mem(0xFE00 + t, r_mem(src + t));
        }
        w_mem(0xFF46, 0xFF);
    }
}

void scnln() {
    if (gt_bt(LCDC, 7)) { // TODO figure out how activation works
        if (LY < SCRN_HEIGHT) {
            if (gt_bt(LCDC, 0)) {
                byte ly = LY;
                int dat_area = gt_bt(LCDC, 4);
                int mp_area = gt_bt(LCDC, 3);
                byte pal = BGP;
                for (int x = 0; x < SCRN_WIDTH; x++) {
                    byte by = (ly + SCY) % 256;
                    int tiley = by / 8;
                    int offy = by % 8;
                    int bytey = offy << 1;
                    int bx = (x + SCX) % 256;
                    int tilex = bx / 8;
                    int offx = bx % 8;
                    int idx = (tiley * 32) + tilex;
                    if (mp_area == 0) idx += 0x9800;
                    else idx += 0x9C00;
                    idx = r_mem(idx);
                    if (dat_area == 0) idx = (signed char)idx + 128;
                    idx *= 16;
                    if (dat_area == 0) idx += 0x8800;
                    else idx += 0x8000;
                    byte ls = r_mem(idx + bytey);
                    byte ms = r_mem(idx + bytey + 1);
                    offx = 7 - offx;
                    int clr = (gt_bt(ms, offx) << 1) | gt_bt(ls, offx);
                    w_pxl(ly, x, gt_clr(pal, clr));
                }
                if (gt_bt(LCDC, 5)) {
                    mp_area = gt_bt(LCDC, 6);
                    byte wx = WX;
                    byte wy = WY;
                    if (wx >= 0 && wx < SCRN_WIDTH + 7 && wy >= 0 && wy < SCRN_HEIGHT && LY >= wy) {
                        wx = wx - 7;
                        wy = WIN_CNT;
                        int tiley = wy / 8;
                        int offy = wy % 8;
                        int bytey = offy << 1;
                        for (byte x = wx; x < SCRN_WIDTH; x++) {
                            byte _wx = x - wx;
                            int tilex = _wx / 8;
                            int offx = _wx % 8;
                            int idx = (tiley * 32) + tilex;
                            if (mp_area == 0) idx += 0x9800;
                            else idx += 0x9C00;
                            idx = r_mem(idx);
                            if (dat_area == 0) idx = (signed char)idx + 128;
                            idx *= 16;
                            if (dat_area == 0) idx += 0x8800;
                            else idx += 0x8000;
                            byte ls = r_mem(idx + bytey);
                            byte ms = r_mem(idx + bytey + 1);
                            offx = 7 - offx;
                            int clr = (gt_bt(ms, offx) << 1) | gt_bt(ls, offx);
                            w_pxl(ly, x, gt_clr(pal, clr));
                        }
                        WIN_CNT++;
                    }
                }
            }
            else {
                for (int x = 0; x < SCRN_WIDTH; x++) {
                    w_pxl(LY, x, CLR_WHT);
                }
            }
            if (gt_bt(LCDC, 1)) {
                byte ly = LY;
                byte sz = gt_bt(LCDC, 2);
                int cnt = 0;
                dbyte obj[10];
                memset(obj, 0, sizeof(obj));
                for (dbyte mem = 0xFE00; mem <= 0xFE9F && cnt < 10; mem += 4) {
                    int y = r_mem(mem + 0);
                    y -= 16;
                    if (ly < y) continue;
                    if (sz) {
                        if (ly >= y + 16) {
                            continue;
                        }
                    }
                    else if (ly >= y + 8) continue;
                    obj[cnt++] = mem;
                }
                dbyte maxx = 0x0100;
                dbyte maxm = 0xFFFF;
                while (cnt--) {
                    int midx = -1;
                    for (int i = 0; i < 10; i++) if (obj[i]) {
                        byte x = r_mem(obj[i] + 1);
                        if (x < maxx || (x == maxx && obj[i] < maxm)) {
                            if (midx == -1) midx = i;
                            else {
                                byte prev = r_mem(obj[midx] + 1);
                                if (x > prev) midx = i;
                                if (x == prev && obj[i] > obj[midx]) midx = i;
                            }
                        }
                    }
                    if (midx == -1) break;
                    dbyte mem = obj[midx];
                    obj[midx] = 0;
                    maxx = r_mem(mem + 1);
                    maxm = mem;
                    int y = r_mem(mem + 0);
                    int x = r_mem(mem + 1);
                    dbyte idx = r_mem(mem + 2);
                    byte  flg = r_mem(mem + 3);
                    y -= 16;
                    x -= 8;
                    if (sz) idx &= 0xFE;
                    idx *= 16;
                    idx += 0x8000;
                    byte flipx = gt_bt(flg, 5);
                    byte flipy = gt_bt(flg, 6);
                    byte line = ly - y;
                    if (sz) {
                        if (flipy) line = 15 - line;
                    }
                    else {
                        if (flipy) line = 7 - line;
                    }
                    line <<= 1;
                    byte ls = r_mem(idx + line + 0);
                    byte ms = r_mem(idx + line + 1);
                    byte pal;
                    if (gt_bt(flg, 4)) pal = OBP1;
                    else pal = OBP0;
                    for (int x0 = x; x0 < x + 8; x0++) {
                        if (x0 < 0) continue;
                        byte posx = 7 - (x0 - x);
                        if (flipx) posx = 7 - posx;
                        byte clr = (gt_bt(ms, posx) << 1) | gt_bt(ls, posx);
                        if (gt_bt(flg, 7)) {
                            if (dsp[ly][x0] == gt_clr(BGP, 0)) w_pxl(ly, x0, gt_clr(pal, clr));
                        }
                        else if (clr != 0) w_pxl(ly, x0, gt_clr(pal, clr));
                    }
                }
            }
        }
    }
    int ly = LY;
    ly++;
    if (ly >= 154) {
        ly = 0;
        WIN_CNT = 0;
        frame = 1;
    }
    w_mem(0xFF44, ly);
}

void redraw() {
    for (int i = 0; i < SCRN_HEIGHT; i++) {
        for (int j = 0; j < SCRN_WIDTH; j++) {
            upd[i][j] = 1;
        }
    }
}

void rndr() {
    for (int i = 0; i < SCRN_HEIGHT; i++) {
        for (int j = 0; j < SCRN_WIDTH; j++) {
            if (upd[i][j]) {
                int clr = dsp[i][j];
                if (clr == CLR_WHT) clr = HEX_WHT;
                if (clr == CLR_L_GRY) clr = HEX_L_GREY;
                if (clr == CLR_D_GRY) clr = HEX_R_GREY;
                if (clr == CLR_BLK) clr = HEX_BLK;
                if (clr == CLR_EXT) clr = HEX_EXT;
                byte r = (clr >> 8 * 2) & 0xFF;
                byte g = (clr >> 8 * 1) & 0xFF;
                byte b = (clr >> 8 * 0) & 0xFF;
                SDL_SetRenderDrawColor(rnd, r, g, b, 0xFF);
                SDL_Rect rct = { j * FCT_X, i * FCT_Y, FCT_X, FCT_Y };
                SDL_RenderFillRect(rnd, &rct);
            }
        }
    }
    SDL_RenderPresent(rnd);
    memset(upd, 0, sizeof(upd));
}

int print_instr(byte instr, byte prfx);
int do_instr();

int do_frame() {
    scn = 0;
    frame = 0;
    int tot_cyc = 0;
    while (!frame) {
        int cyc = do_instr();
        if (cyc == -1) return -1;
        PC++;
        scn += cyc;
        tot_cyc += cyc;
        if (scn >= 456) {
            scnln();
            scn -= 456;
        }
        upd_lcd();
        upd_tim(cyc);
        if (handle_in()) return -1;
        upd_dma();
        chck_intr();
        // if (PC >= 0x100) dbg();
    }
    rndr();
    return tot_cyc;
}

int main(int argc, char* argv[]) {
    // freopen("logfile.txt", "w", stdout);
    init_reg();
    FILE* boot_rom_file;
    FILE* rom_file;
    fopen_s(&boot_rom_file, "bootrom.rom", "rb");
    fopen_s(&rom_file, rom_name, "rb");
    int loop = 1;
    if (!boot_rom_file) {
        printf("COULD NOT OPEN BOOT ROM\n");
        loop = 0;
    }
    else fread(brom, 0x100, 1, boot_rom_file);
    if (!rom_file) {
        printf("COULD NOT OPEN ROM\n");
        loop = 0;
    }
    else fread(mem, 0x8000, 1, rom_file); // Put only first bank in memory
    if (cart_info()) loop = 0;
    fopen_s(&rom_file, rom_name, "rb");
    if (!rom_file);
    else fread(rom, rom_size_bytes, 1, rom_file);
    if (loop && init_dsp()) loop = 0;
    while (loop) {
        Uint32 start = SDL_GetTicks();
        int cyc = do_frame();
        Uint32 end = SDL_GetTicks();
        if (cyc == -1) {
            loop = 0;
            continue;
        }
        Uint32 time_needed = (Uint32) (((float)cyc / (float)CPU_FREQ) * 1000.0);
        Uint32 time_elapsed = end - start;
        if (time_elapsed < time_needed) SDL_Delay(time_needed - time_elapsed);
        else printf("PERFORMANCE DIP\n");
    }
    dbg();
    printf("DONE\n");
    return 0;
}

int print_instr(byte instr, byte prfx) {
    if (!r_mem(0xFF50)) return 0;
    printf("$%04X %02X ", PC, instr);
    if (instr == 0xCB) {
        printf("%02X ", prfx);
        switch (prfx) {
        case 0x11:
            printf("RL C\n");
            break;
        case 0x19:
            printf("RR C\n");
            break;
        case 0x1A:
            printf("RR D\n");
            break;
        case 0x20:
            printf("SLA B\n");
            break;
        case 0x37:
            printf("SWAP A\n");
            break;
        case 0x38:
            printf("SRL B\n");
            break;
        case 0x50:
            printf("BIT 2, B\n");
            break;
        case 0x58:
            printf("BIT 3, B\n");
            break;
        case 0x6C:
            printf("BIT 5, H\n");
        case 0x70:
            printf("BIT 6, B\n");
            break;
        case 0x78:
            printf("BIT 7, B\n");
            break;
        case 0x7C:
            printf("BIT 7, H\n");
            break;
        case 0x7E:
            printf("BIT 7, (HL)\n");
            break;
        case 0x86:
            printf("RES 0, (HL)\n");
            break;
        case 0x87:
            printf("RES 0, A\n");
            break;
        case 0x8E:
            printf("RES 1, (HL)\n");
            break;
        case 0x96:
            printf("RES 2, (HL)\n");
            break;
        case 0x9E:
            printf("RES 3, (HL)\n");
            break;
        case 0xA6:
            printf("RES 4, (HL)\n");
            break;
        case 0xAE:
            printf("RES 5, (HL)\n");
            break;
        case 0xB6:
            printf("RES 6, (HL)\n");
            break;
        case 0xC6:
            printf("SET 0, (HL)\n");
            break;
        case 0xCE:
            printf("SET 1, (HL)\n");
            break;
        case 0xD6:
            printf("SET 2, (HL)\n");
            break;
        case 0xDE:
            printf("SET 3, (HL)\n");
            break;
        case 0xE6:
            printf("SET 4, (HL)\n");
            break;
        case 0xEE:
            printf("SET 5, (HL)\n");
            break;
        case 0xF6:
            printf("SET 6, (HL)\n");
            break;
        case 0xFE:
            printf("SET 7, (HL)\n");
            break;
        default:
            printf("UNKONWN PREFIX INSTRUCTION %02X\n", prfx);
            return 1;
        }
    }
    else {
        switch (instr) {
        case 0x00:
            printf("NOP\n");
            break;
        case 0x01:
            printf("LD BC, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0x02:
            printf("LD (BC), A\n");
            break;
        case 0x03:
            printf("INC BC\n");
            break;
        case 0x04:
            printf("INC B\n");
            break;
        case 0x05:
            printf("DEC B\n");
            break;
        case 0x06:
            printf("LD B, $%02X\n", rd8());
            PC--;
            break;
        case 0x07:
            printf("RLCA\n");
            break;
        case 0x08:
            printf("LD ($%04X), SP\n", rd16());
            PC -= 2;
            break;
        case 0x09:
            printf("ADD HL, BC\n");
            break;
        case 0x0A:
            printf("LD A, (BC)\n");
            break;
        case 0x0B:
            printf("DEC BC\n");
            break;
        case 0x0C:
            printf("INC C\n");
            break;
        case 0x0D:
            printf("DEC C\n");
            break;
        case 0x0E:
            printf("LD C, $%02X\n", rd8());
            PC--;
            break;
        case 0x0F:
            printf("RRCA\n");
            break;
        case 0x10:
            printf("STOP\n");
            break;
        case 0x11:
            printf("LD DE, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0x12:
            printf("LD (DE), A\n");
            break;
        case 0x13:
            printf("INC DE\n");
            break;
        case 0x14:
            printf("INC D\n");
            break;
        case 0x15:
            printf("DEC D\n");
            break;
        case 0x16:
            printf("LD D, $%02X\n", rd8());
            PC--;
            break;
        case 0x17:
            printf("RLA\n");
            break;
        case 0x18:
            printf("JR %d\n", (signed char)rd8());
            PC--;
            break;
        case 0x19:
            printf("ADD HL, DE\n");
            break;
        case 0x1A:
            printf("LD A, (DE)\n");
            break;
        case 0x1B:
            printf("DEC DE\n");
            break;
        case 0x1C:
            printf("INC E\n");
            break;
        case 0x1D:
            printf("DEC E\n");
            break;
        case 0x1E:
            printf("LD E, $%02X\n", rd8());
            PC--;
            break;
        case 0x1F:
            printf("RRA\n");
            break;
        case 0x20:
            printf("JR NZ, %d\n", (signed char)rd8());
            PC--;
            break;
        case 0x21:
            printf("LD HL, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0x22:
            printf("LD (HL+), A\n");
            break;
        case 0x23:
            printf("INC HL\n");
            break;
        case 0x24:
            printf("INC H\n");
            break;
        case 0x25:
            printf("DEC H\n");
            break;
        case 0x26:
            printf("LD H, $%02X\n", rd8());
            PC--;
            break;
        case 0x27:
            printf("DAA\n");
            break;
        case 0x28:
            printf("JR Z, %d\n", (signed char)rd8());
            PC--;
            break;
        case 0x29:
            printf("ADD HL, HL\n");
            break;
        case 0x2A:
            printf("LD A, (HL+)\n");
            break;
        case 0x2B:
            printf("DEC HL\n");
            break;
        case 0x2C:
            printf("INC L\n");
            break;
        case 0x2D:
            printf("DEC L\n");
            break;
        case 0x2E:
            printf("LD L, $%02X\n", rd8());
            PC--;
            break;
        case 0x2F:
            printf("CPL\n");
            break;
        case 0x30:
            printf("JR NC, %d\n", (signed char)rd8());
            PC--;
            break;
        case 0x31:
            printf("LD SP $%04X\n", rd16());
            PC -= 2;
            break;
        case 0x32:
            printf("LD (HL-), A\n");
            break;
        case 0x33:
            printf("INC SP\n");
            break;
        case 0x34:
            printf("INC (HL)\n");
            break;
        case 0x35:
            printf("DEC (HL)\n");
            break;
        case 0x36:
            printf("LD (HL), $%02X\n", rd8());
            PC--;
            break;
        case 0x37:
            printf("SCF\n");
            break;
        case 0x38:
            printf("JR C, %d\n", (signed char)rd8());
            PC--;
            break;
        case 0x39:
            printf("ADD HL, SP\n");
            break;
        case 0x3A:
            printf("LD A, (HL-)\n");
            break;
        case 0x3B:
            printf("DEC SP\n");
            break;
        case 0x3C:
            printf("INC A\n");
            break;
        case 0x3D:
            printf("DEC A\n");
            break;
        case 0x3E:
            printf("LD A, $%02X\n", rd8());
            PC--;
            break;
        case 0x3F:
            printf("CCF\n");
            break;
        case 0x40:
            printf("LD B, B\n");
            break;
        case 0x41:
            printf("LD B, C\n");
            break;
        case 0x42:
            printf("LD B, D\n");
            break;
        case 0x43:
            printf("LD B, E\n");
            break;
        case 0x44:
            printf("LD B, H\n");
            break;
        case 0x45:
            printf("LD B, L\n");
            break;
        case 0x46:
            printf("LD B, (HL)\n");
            break;
        case 0x47:
            printf("LD B, A\n");
            break;
        case 0x48:
            printf("LD C, B\n");
            break;
        case 0x49:
            printf("LD C, C\n");
            break;
        case 0x4A:
            printf("LD C, D\n");
            break;
        case 0x4B:
            printf("LD C, E\n");
            break;
        case 0x4C:
            printf("LD C, H\n");
            break;
        case 0x4D:
            printf("LD C, L\n");
            break;
        case 0x4E:
            printf("LD C, (HL)\n");
            break;
        case 0x4F:
            printf("LD C, A\n");
            break;
        case 0x50:
            printf("LD D, B\n");
            break;
        case 0x51:
            printf("LD D, C\n");
            break;
        case 0x52:
            printf("LD D, D\n");
            break;
        case 0x53:
            printf("LD D, E\n");
            break;
        case 0x54:
            printf("LD D, H\n");
            break;
        case 0x55:
            printf("LD D, L\n");
            break;
        case 0x56:
            printf("LD D, (HL)\n");
            break;
        case 0x57:
            printf("LD D, A\n");
            break;
        case 0x58:
            printf("LD E, B\n");
            break;
        case 0x59:
            printf("LD E, C\n");
            break;
        case 0x5A:
            printf("LD E, D\n");
            break;
        case 0x5B:
            printf("LD E, E\n");
            break;
        case 0x5C:
            printf("LD E, H\n");
            break;
        case 0x5D:
            printf("LD E, L\n");
            break;
        case 0x5E:
            printf("LD E, (HL)\n");
            break;
        case 0x5F:
            printf("LD E, A\n");
            break;
        case 0x60:
            printf("LD H, B\n");
            break;
        case 0x61:
            printf("LD H, C\n");
            break;
        case 0x62:
            printf("LD H, D\n");
            break;
        case 0x63:
            printf("LD H, E\n");
            break;
        case 0x64:
            printf("LD H, H\n");
            break;
        case 0x65:
            printf("LD H, L\n");
            break;
        case 0x66:
            printf("LD H, (HL)\n");
            break;
        case 0x67:
            printf("LD H, A\n");
            break;
        case 0x68:
            printf("LD L, B\n");
            break;
        case 0x69:
            printf("LD L, C\n");
            break;
        case 0x6A:
            printf("LD L, D\n");
            break;
        case 0x6B:
            printf("LD L, E\n");
            break;
        case 0x6C:
            printf("LD L, H\n");
            break;
        case 0x6D:
            printf("LD L, L\n");
            break;
        case 0x6E:
            printf("LD L, (HL)\n");
            break;
        case 0x6F:
            printf("LD L, A\n");
            break;
        case 0x70:
            printf("LD (HL), B\n");
            break;
        case 0x71:
            printf("LD (HL), C\n");
            break;
        case 0x72:
            printf("LD (HL), D\n");
            break;
        case 0x76:
            printf("HALT\n");
            break;
        case 0x77:
            printf("LD (HL), A\n");
            break;
        case 0x78:
            printf("LD A, B\n");
            break;
        case 0x79:
            printf("LD A, C\n");
            break;
        case 0x7A:
            printf("LD A, D\n");
            break;
        case 0x7B:
            printf("LD A, E\n");
            break;
        case 0x7C:
            printf("LD A, H\n");
            break;
        case 0x7D:
            printf("LD A, L\n");
            break;
        case 0x7E:
            printf("LD A, (HL)\n");
            break;
        case 0x7F:
            printf("LD A, A\n");
            break;
        case 0x80:
            printf("ADD A, B\n");
            break;
        case 0x81:
            printf("ADD A, C\n");
            break;
        case 0x82:
            printf("ADD A, D\n");
            break;
        case 0x83:
            printf("ADD A, E\n");
            break;
        case 0x84:
            printf("ADD A, H\n");
            break;
        case 0x85:
            printf("ADD A, L\n");
            break;
        case 0x86:
            printf("ADD A, (HL)\n");
            break;
        case 0x87:
            printf("ADD A, A\n");
            break;
        case 0x88:
            printf("ADC A, B\n");
            break;
        case 0x89:
            printf("ADC A, C\n");
            break;
        case 0x8A:
            printf("ADC A, D\n");
            break;
        case 0x8B:
            printf("ADC A, E\n");
            break;
        case 0x8C:
            printf("ADC A, H\n");
            break;
        case 0x8D:
            printf("ADC A, L\n");
            break;
        case 0x8E:
            printf("ADC A, (HL)\n");
            break;
        case 0x8F:
            printf("ADC A, A\n");
            break;
        case 0x90:
            printf("SUB B\n");
            break;
        case 0xA1:
            printf("AND C\n");
            break;
        case 0xA7:
            printf("AND A\n");
            break;
        case 0xA9:
            printf("XOR C\n");
            break;
        case 0xAE:
            printf("XOR (HL)\n");
            break;
        case 0xAF:
            printf("XOR A\n");
            break;
        case 0xB0:
            printf("OR B\n");
            break;
        case 0xB1:
            printf("OR C\n");
            break;
        case 0xB6:
            printf("OR (HL)\n");
            break;
        case 0xB7:
            printf("OR A\n");
            break;
        case 0xB8:
            printf("CP B\n");
            break;
        case 0xBB:
            printf("CP E\n");
            break;
        case 0xBE:
            printf("CP (HL)\n");
            break;
        case 0xBF:
            printf("CP A\n");
            break;
        case 0xC0:
            printf("RET NZ\n");
            break;
        case 0xC1:
            printf("POP BC\n");
            break;
        case 0xC2:
            printf("JP NZ $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xC3:
            printf("JP $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xC4:
            printf("CALL NZ, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xC5:
            printf("PUSH BC\n");
            break;
        case 0xC6:
            printf("ADD A, $%02X\n", rd8());
            PC--;
            break;
        case 0xC8:
            printf("RET Z\n");
            break;
        case 0xC9:
            printf("RET\n");
            break;
        case 0xCA:
            printf("JP Z, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xCD:
            printf("CALL $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xD1:
            printf("POP DE\n");
            break;
        case 0xD2:
            printf("JP NC, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xD5:
            printf("PUSH DE\n");
            break;
        case 0xD6:
            printf("SUB $%02X\n", rd8());
            PC--;
            break;
        case 0xD9:
            printf("RETI\n");
            break;
        case 0xDA:
            printf("JP C $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xDF:
            printf("RST 3\n");
            break;
        case 0xE0:
            printf("LD ($FF00+%02X), A\n", rd8());
            PC--;
            break;
        case 0xE1:
            printf("POP HL\n");
            break;
        case 0xE2:
            printf("LD ($FF00+C), A\n");
            break;
        case 0xE5:
            printf("PUSH HL\n");
            break;
        case 0xE6:
            printf("AND $%02X\n", rd8());
            PC--;
            break;
        case 0xE7:
            printf("RST 4\n");
            break;
        case 0xE9:
            printf("JP HL\n");
            break;
        case 0xEA:
            printf("LD ($%04X), A\n", rd16());
            PC -= 2;
            break;
        case 0xEE:
            printf("XOR $%02X\n", rd8());
            PC--;
            break;
        case 0xEF:
            printf("RST 5\n");
            break;
        case 0xF0:
            printf("LD A, ($FF00+$%02X)\n", rd8());
            PC--;
            break;
        case 0xF1:
            printf("POP AF\n");
            break;
        case 0xF3:
            printf("DI\n");
            break;
        case 0xF5:
            printf("PUSH AF\n");
            break;
        case 0xF6:
            printf("OR $%02X\n", rd8());
            PC--;
            break;
        case 0xF9:
            printf("LD SP, HL\n");
            break;
        case 0xFA:
            printf("LD A, $%04X\n", rd16());
            PC -= 2;
            break;
        case 0xFB:
            printf("EI\n");
            break;
        case 0xFE:
            printf("CP $%02X\n", rd8());
            PC--;
            break;
        case 0xFF:
            printf("RST 7\n");
            break;
        default:
            printf("UNKNOWN INSTRUCTION %02X\n", instr);
            return 1;
        }
    }
    return 0;
}

int do_instr() {
    if (HALT) {
        kp();
        return 4;
    }
    byte instr = r_mem(PC);
    if (PC >= 0x8000 && 0) { // Unset for debugging
        printf("PROGRAM COUNTER PAST CARTRIDGE SPACE\n");
        return -1;
    }
    if (instr == 0xCB) {
        byte prfx = rd8();
        if (dis) print_instr(instr, prfx);
        switch (prfx) {
        case 0x00:
            return c_rlc(&B);
        case 0x01:
            return c_rlc(&C);
        case 0x02:
            return c_rlc(&D);
        case 0x03:
            return c_rlc(&E);
        case 0x04:
            return c_rlc(&H);
        case 0x05:
            return c_rlc(&L);
        case 0x06:
            return c_rlc_mem(gt_HL());
        case 0x07:
            return c_rlc(&A);
        case 0x08:
            return c_rrc(&B);
        case 0x09:
            return c_rrc(&C);
        case 0x0A:
            return c_rrc(&D);
        case 0x0B:
            return c_rrc(&E);
        case 0x0C:
            return c_rrc(&H);
        case 0x0D:
            return c_rrc(&L);
        case 0x0E:
            return c_rrc_mem(gt_HL());
        case 0x0F:
            return c_rrc(&A);
        case 0x10:
            return c_rl(&B);
        case 0x11:
            return c_rl(&C);
        case 0x12:
            return c_rl(&D);
        case 0x13:
            return c_rl(&E);
        case 0x14:
            return c_rl(&H);
        case 0x15:
            return c_rl(&L);
        case 0x16:
            return c_rl_mem(gt_HL());
        case 0x17:
            return c_rl(&A);
        case 0x18:
            return c_rr(&B);
        case 0x19:
            return c_rr(&C);
        case 0x1A:
            return c_rr(&D);
        case 0x1B:
            return c_rr(&E);
        case 0x1C:
            return c_rr(&H);
        case 0x1D:
            return c_rr(&L);
        case 0x1E:
            return c_rr_mem(gt_HL());
        case 0x1F:
            return c_rr(&A);
        case 0x20:
            return c_sla(&B);
        case 0x21:
            return c_sla(&C);
        case 0x22:
            return c_sla(&D);
        case 0x23:
            return c_sla(&E);
        case 0x24:
            return c_sla(&H);
        case 0x25:
            return c_sla(&L);
        case 0x26:
            return c_sla_mem(gt_HL());
        case 0x27:
            return c_sla(&A);
        case 0x28:
            return c_sra(&B);
        case 0x29:
            return c_sra(&C);
        case 0x2A:
            return c_sra(&D);
        case 0x2B:
            return c_sra(&E);
        case 0x2C:
            return c_sra(&H);
        case 0x2D:
            return c_sra(&L);
        case 0x2E:
            return c_sra_mem(gt_HL());
        case 0x2F:
            return c_sra(&A);
        case 0x30:
            return c_swp(&B);
        case 0x31:
            return c_swp(&C);
        case 0x32:
            return c_swp(&D);
        case 0x33:
            return c_swp(&E);
        case 0x34:
            return c_swp(&H);
        case 0x35:
            return c_swp(&L);
        case 0x36:
            return c_swp_mem(gt_HL());
        case 0x37:
            return c_swp(&A);
        case 0x38:
            return c_srl(&B);
        case 0x39:
            return c_srl(&C);
        case 0x3A:
            return c_srl(&D);
        case 0x3B:
            return c_srl(&E);
        case 0x3C:
            return c_srl(&H);
        case 0x3D:
            return c_srl(&L);
        case 0x3E:
            return c_srl_mem(gt_HL());
        case 0x3F:
            return c_srl(&A);
        case 0x40:
            return c_bit(B, 0);
        case 0x41:
            return c_bit(C, 0);
        case 0x42:
            return c_bit(D, 0);
        case 0x43:
            return c_bit(E, 0);
        case 0x44:
            return c_bit(H, 0);
        case 0x45:
            return c_bit(L, 0);
        case 0x46:
            c_bit(r_mem(gt_HL()), 0);
            return 12;
        case 0x47:
            return c_bit(A, 0);
        case 0x48:
            return c_bit(B, 1);
        case 0x49:
            return c_bit(C, 1);
        case 0x4A:
            return c_bit(D, 1);
        case 0x4B:
            return c_bit(E, 1);
        case 0x4C:
            return c_bit(H, 1);
        case 0x4D:
            return c_bit(L, 1);
        case 0x4E:
            c_bit(r_mem(gt_HL()), 1);
            return 12;
        case 0x4F:
            return c_bit(A, 1);
        case 0x50:
            return c_bit(B, 2);
        case 0x51:
            return c_bit(C, 2);
        case 0x52:
            return c_bit(D, 2);
        case 0x53:
            return c_bit(E, 2);
        case 0x54:
            return c_bit(H, 2);
        case 0x55:
            return c_bit(L, 2);
        case 0x56:
            c_bit(r_mem(gt_HL()), 2);
            return 12;
        case 0x57:
            return c_bit(A, 2);
        case 0x58:
            return c_bit(B, 3);
        case 0x59:
            return c_bit(C, 3);
        case 0x5A:
            return c_bit(D, 3);
        case 0x5B:
            return c_bit(E, 3);
        case 0x5C:
            return c_bit(H, 3);
        case 0x5D:
            return c_bit(L, 3);
        case 0x5E:
            c_bit(r_mem(gt_HL()), 3);
            return 12;
        case 0x5F:
            return c_bit(A, 3);
        case 0x60:
            return c_bit(B, 4);
        case 0x61:
            return c_bit(C, 4);
        case 0x62:
            return c_bit(D, 4);
        case 0x63:
            return c_bit(E, 4);
        case 0x64:
            return c_bit(H, 4);
        case 0x65:
            return c_bit(L, 4);
        case 0x66:
            c_bit(r_mem(gt_HL()), 4);
            return 12;
        case 0x67:
            return c_bit(A, 4);
        case 0x68:
            return c_bit(B, 5);
        case 0x69:
            return c_bit(C, 5);
        case 0x6A:
            return c_bit(D, 5);
        case 0x6B:
            return c_bit(E, 5);
        case 0x6C:
            return c_bit(H, 5);
        case 0x6D:
            return c_bit(L, 5);
        case 0x6E:
            c_bit(r_mem(gt_HL()), 5);
            return 12;
        case 0x6F:
            return c_bit(A, 5);
        case 0x70:
            return c_bit(B, 6);
        case 0x71:
            return c_bit(C, 6);
        case 0x72:
            return c_bit(D, 6);
        case 0x73:
            return c_bit(E, 6);
        case 0x74:
            return c_bit(H, 6);
        case 0x75:
            return c_bit(L, 6);
        case 0x76:
            c_bit(r_mem(gt_HL()), 6);
            return 12;
        case 0x77:
            return c_bit(A, 6);
        case 0x78:
            return c_bit(B, 7);
        case 0x79:
            return c_bit(C, 7);
        case 0x7A:
            return c_bit(D, 7);
        case 0x7B:
            return c_bit(E, 7);
        case 0x7C:
            return c_bit(H, 7);
        case 0x7D:
            return c_bit(L, 7);
        case 0x7E:
            c_bit(r_mem(gt_HL()), 7);
            return 12;
        case 0x7F:
            return c_bit(A, 7);
        case 0x80:
            return c_res(&B, 0);
        case 0x81:
            return c_res(&C, 0);
        case 0x82:
            return c_res(&D, 0);
        case 0x83:
            return c_res(&E, 0);
        case 0x84:
            return c_res(&H, 0);
        case 0x85:
            return c_res(&L, 0);
        case 0x86:
            return c_res_mem(gt_HL(), 0);
        case 0x87:
            return c_res(&A, 0);
        case 0x88:
            return c_res(&B, 1);
        case 0x89:
            return c_res(&C, 1);
        case 0x8A:
            return c_res(&D, 1);
        case 0x8B:
            return c_res(&E, 1);
        case 0x8C:
            return c_res(&H, 1);
        case 0x8D:
            return c_res(&L, 1);
        case 0x8E:
            return c_res_mem(gt_HL(), 1);
        case 0x8F:
            return c_res(&A, 1);
        case 0x90:
            return c_res(&B, 2);
        case 0x91:
            return c_res(&C, 2);
        case 0x92:
            return c_res(&D, 2);
        case 0x93:
            return c_res(&E, 2);
        case 0x94:
            return c_res(&H, 2);
        case 0x95:
            return c_res(&L, 2);
        case 0x96:
            return c_res_mem(gt_HL(), 2);
        case 0x97:
            return c_res(&A, 2);
        case 0x98:
            return c_res(&B, 3);
        case 0x99:
            return c_res(&C, 3);
        case 0x9A:
            return c_res(&D, 3);
        case 0x9B:
            return c_res(&E, 3);
        case 0x9C:
            return c_res(&H, 3);
        case 0x9D:
            return c_res(&L, 3);
        case 0x9E:
            return c_res_mem(gt_HL(), 3);
        case 0x9F:
            return c_res(&A, 3);
        case 0xA0:
            return c_res(&B, 4);
        case 0xA1:
            return c_res(&C, 4);
        case 0xA2:
            return c_res(&D, 4);
        case 0xA3:
            return c_res(&E, 4);
        case 0xA4:
            return c_res(&H, 4);
        case 0xA5:
            return c_res(&L, 4);
        case 0xA6:
            return c_res_mem(gt_HL(), 4);
        case 0xA7:
            return c_res(&A, 4);
        case 0xA8:
            return c_res(&B, 5);
        case 0xA9:
            return c_res(&C, 5);
        case 0xAA:
            return c_res(&D, 5);
        case 0xAB:
            return c_res(&E, 5);
        case 0xAC:
            return c_res(&H, 5);
        case 0xAD:
            return c_res(&L, 5);
        case 0xAE:
            return c_res_mem(gt_HL(), 5);
        case 0xAF:
            return c_res(&A, 5);
        case 0xB0:
            return c_res(&B, 6);
        case 0xB1:
            return c_res(&C, 6);
        case 0xB2:
            return c_res(&D, 6);
        case 0xB3:
            return c_res(&E, 6);
        case 0xB4:
            return c_res(&H, 6);
        case 0xB5:
            return c_res(&L, 6);
        case 0xB6:
            return c_res_mem(gt_HL(), 6);
        case 0xB7:
            return c_res(&A, 6);
        case 0xB8:
            return c_res(&B, 7);
        case 0xB9:
            return c_res(&C, 7);
        case 0xBA:
            return c_res(&D, 7);
        case 0xBB:
            return c_res(&E, 7);
        case 0xBC:
            return c_res(&H, 7);
        case 0xBD:
            return c_res(&L, 7);
        case 0xBE:
            return c_res_mem(gt_HL(), 7);
        case 0xBF:
            return c_res(&A, 7);
        case 0xC0:
            return c_set(&B, 0);
        case 0xC1:
            return c_set(&C, 0);
        case 0xC2:
            return c_set(&D, 0);
        case 0xC3:
            return c_set(&E, 0);
        case 0xC4:
            return c_set(&H, 0);
        case 0xC5:
            return c_set(&L, 0);
        case 0xC6:
            return c_set_mem(gt_HL(), 0);
        case 0xC7:
            return c_set(&A, 0);
        case 0xC8:
            return c_set(&B, 1);
        case 0xC9:
            return c_set(&C, 1);
        case 0xCA:
            return c_set(&D, 1);
        case 0xCB:
            return c_set(&E, 1);
        case 0xCC:
            return c_set(&H, 1);
        case 0xCD:
            return c_set(&L, 1);
        case 0xCE:
            return c_set_mem(gt_HL(), 1);
        case 0xCF:
            return c_set(&A, 1);
        case 0xD0:
            return c_set(&B, 2);
        case 0xD1:
            return c_set(&C, 2);
        case 0xD2:
            return c_set(&D, 2);
        case 0xD3:
            return c_set(&E, 2);
        case 0xD4:
            return c_set(&H, 2);
        case 0xD5:
            return c_set(&L, 2);
        case 0xD6:
            return c_set_mem(gt_HL(), 2);
        case 0xD7:
            return c_set(&A, 2);
        case 0xD8:
            return c_set(&B, 3);
        case 0xD9:
            return c_set(&C, 3);
        case 0xDA:
            return c_set(&D, 3);
        case 0xDB:
            return c_set(&E, 3);
        case 0xDC:
            return c_set(&H, 3);
        case 0xDD:
            return c_set(&L, 3);
        case 0xDE:
            c_set_mem(gt_HL(), 3);
            return 16;
        case 0xDF:
            return c_set(&A, 3);
        case 0xE0:
            return c_set(&B, 4);
        case 0xE1:
            return c_set(&C, 4);
        case 0xE2:
            return c_set(&D, 4);
        case 0xE3:
            return c_set(&E, 4);
        case 0xE4:
            return c_set(&H, 4);
        case 0xE5:
            return c_set(&L, 4);
        case 0xE6:
            c_set_mem(gt_HL(), 4);
            return 16;
        case 0xE7:
            return c_set(&A, 4);
        case 0xE8:
            return c_set(&B, 5);
        case 0xE9:
            return c_set(&C, 5);
        case 0xEA:
            return c_set(&D, 5);
        case 0xEB:
            return c_set(&E, 5);
        case 0xEC:
            return c_set(&H, 5);
        case 0xED:
            return c_set(&L, 5);
        case 0xEE:
            c_set_mem(gt_HL(), 5);
            return 16;
        case 0xEF:
            return c_set(&A, 5);
        case 0xF0:
            return c_set(&B, 6);
        case 0xF1:
            return c_set(&C, 6);
        case 0xF2:
            return c_set(&D, 6);
        case 0xF3:
            return c_set(&E, 6);
        case 0xF4:
            return c_set(&H, 6);
        case 0xF5:
            return c_set(&L, 6);
        case 0xF6:
            c_set_mem(gt_HL(), 6);
            return 16;
        case 0xF7:
            return c_set(&A, 6);
        case 0xF8:
            return c_set(&B, 7);
        case 0xF9:
            return c_set(&C, 7);
        case 0xFA:
            return c_set(&D, 7);
        case 0xFB:
            return c_set(&E, 7);
        case 0xFC:
            return c_set(&H, 7);
        case 0xFD:
            return c_set(&L, 7);
        case 0xFE:
            c_set_mem(gt_HL(), 7);
            return 16;
        case 0xFF:
            return c_set(&A, 7);
        default:
            printf("UNIMPLEMENTED PREFIX INSTRUCTION\n");
            print_instr(instr, prfx);
            return -1;
        }
    }
    else {
        if (dis) print_instr(instr, 0);
        switch (instr) {
        case 0x00:
            return 4;
        case 0x01:
            st_BC(rd16());
            return 12;
        case 0x02:
            w_mem(gt_BC(), A);
            return 8;
        case 0x03:
            st_BC(gt_BC() + 1);
            return 8;
        case 0x04:
            return c_inc(&B);
        case 0x05:
            return c_dec(&B);
        case 0x06:
            B = rd8();
            return 8;
        case 0x07:
            c_rlc(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x08:
        {
            dbyte addr = rd16();
            w_mem(addr, (byte)(SP & 0xFF));
            w_mem(addr + 1, (byte)((SP >> 8) & 0xFF));
            return 20;
        }
        case 0x09:
            st_h_add16(gt_HL(), gt_BC());
            st_c_add16(gt_HL(), gt_BC());
            st_HL(gt_HL() + gt_BC());
            cl_flg(FLG_N);
            return 8;
        case 0x0A:
            A = r_mem(gt_BC());
            return 8;
        case 0x0B:
            st_BC(gt_BC() - 1);
            return 8;
        case 0x0C:
            return c_inc(&C);
        case 0x0D:
            return c_dec(&C);
        case 0x0E:
            C = rd8();
            return 8;
        case 0x0F:
            c_rrc(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x10:
            return 4;
        case 0x11:
            st_DE(rd16());
            return 12;
        case 0x12:
            w_mem(gt_DE(), A);
            return 8;
        case 0x13:
            st_DE(gt_DE() + 1);
            return 8;
        case 0x14:
            return c_inc(&D);
        case 0x15:
            return c_dec(&D);
        case 0x16:
            D = rd8();
            return 8;
        case 0x17:
            c_rl(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x18:
            PC += (signed char)rd8();
            return 12;
        case 0x19:
            st_h_add16(gt_HL(), gt_DE());
            st_c_add16(gt_HL(), gt_DE());
            st_HL(gt_HL() + gt_DE());
            cl_flg(FLG_N);
            return 8;
        case 0x1A:
            A = r_mem(gt_DE());
            return 8;
        case 0x1B:
            st_DE(gt_DE() - 1);
            return 8;
        case 0x1C:
            return c_inc(&E);
        case 0x1D:
            return c_dec(&E);
        case 0x1E:
            E = rd8();
            return 8;
        case 0x1F:
            c_rr(&A);
            cl_flg(FLG_Z);
            return 4;
        case 0x20:
            return c_jp8(1 - gt_flg(FLG_Z));
        case 0x21:
            st_HL(rd16());
            return 12;
        case 0x22:
            w_mem(gt_HL(), A);
            st_HL(gt_HL() + 1);
            return 8;
        case 0x23:
            st_HL(gt_HL() + 1);
            return 8;
        case 0x24:
            return c_inc(&H);
        case 0x25:
            return c_dec(&H);
        case 0x26:
            H = rd8();
            return 8;
        case 0x27: // No clue how this works I just copied it
            if (!gt_flg(FLG_N)) {
                if (gt_flg(FLG_C) || A > 0x99) {
                    A += 0x60;
                    st_flg(FLG_C);
                }
                if (gt_flg(FLG_H) || (A & 0x0f) > 0x09) {
                    A += 0x06;
                }
            }
            else {
                if (gt_flg(FLG_C)) {
                    A -= 0x60;
                }
                if (gt_flg(FLG_H)) {
                    A -= 0x06;
                }
            }
            st_z(A);
            cl_flg(FLG_H);
            return 4;
        case 0x28:
            return c_jp8(gt_flg(FLG_Z));
        case 0x29:
            st_h_add16(gt_HL(), gt_HL());
            st_c_add16(gt_HL(), gt_HL());
            st_HL(gt_HL() + gt_HL());
            cl_flg(FLG_N);
            return 8;
        case 0x2A:
            A = r_mem(gt_HL());
            st_HL(gt_HL() + 1);
            return 8;
        case 0x2B:
            st_HL(gt_HL() - 1);
            return 8;
        case 0x2C:
            return c_inc(&L);
        case 0x2D:
            return c_dec(&L);
        case 0x2E:
            L = rd8();
            return 8;
        case 0x2F:
            return c_cpl(&A);
        case 0x30:
            return c_jp8(1 - gt_flg(FLG_C));
        case 0x31:
            SP = rd16();
            return 12;
        case 0x32:
            w_mem(gt_HL(), A);
            st_HL(gt_HL() - 1);
            return 8;
        case 0x33:
            ++SP;
            return 8;
        case 0x34:
            return c_inc_mem(gt_HL());
        case 0x35:
            return c_dec_mem(gt_HL());
        case 0x36:
            w_mem(gt_HL(), rd8());
            return 12;
        case 0x37:
            cl_flg(FLG_N);
            cl_flg(FLG_H);
            st_flg(FLG_C);
            return 4;
        case 0x38:
            return c_jp8(gt_flg(FLG_C));
        case 0x39:
            st_h_add16(gt_HL(), SP);
            st_c_add16(gt_HL(), SP);
            st_HL(gt_HL() + SP);
            cl_flg(FLG_N);
            return 8;
        case 0x3A:
            A = r_mem(gt_HL());
            st_HL(gt_HL() - 1);
            return 8;
        case 0x3B:
            --SP;
            return 8;
        case 0x3C:
            return c_inc(&A);
        case 0x3D:
            return c_dec(&A);
        case 0x3E:
            A = rd8();
            return 8;
        case 0x3F:
            cl_flg(FLG_N);
            cl_flg(FLG_H);
            if (gt_flg(FLG_C)) cl_flg(FLG_C);
            else st_flg(FLG_C);
            return 4;
        case 0x40:
            B = B;
            return 4;
        case 0x41:
            B = C;
            return 4;
        case 0x42:
            B = D;
            return 4;
        case 0x43:
            B = E;
            return 4;
        case 0x44:
            B = H;
            return 4;
        case 0x45:
            B = L;
            return 4;
        case 0x46:
            B = r_mem(gt_HL());
            return 8;
        case 0x47:
            B = A;
            return 4;
        case 0x48:
            C = B;
            return 4;
        case 0x49:
            C = C;
            return 4;
        case 0x4A:
            C = D;
            return 4;
        case 0x4B:
            C = E;
            return 4;
        case 0x4C:
            C = H;
            return 4;
        case 0x4D:
            C = L;
            return 4;
        case 0x4E:
            C = r_mem(gt_HL());
            return 8;
        case 0x4F:
            C = A;
            return 4;
        case 0x50:
            D = B;
            return 4;
        case 0x51:
            D = C;
            return 4;
        case 0x52:
            D = D;
            return 4;
        case 0x53:
            D = E;
            return 4;
        case 0x54:
            D = H;
            return 4;
        case 0x55:
            D = L;
            return 4;
        case 0x56:
            D = r_mem(gt_HL());
            return 8;
        case 0x57:
            D = A;
            return 4;
        case 0x58:
            E = B;
            return 4;
        case 0x59:
            E = C;
            return 4;
        case 0x5A:
            E = D;
            return 4;
        case 0x5B:
            E = E;
            return 4;
        case 0x5C:
            E = H;
            return 4;
        case 0x5D:
            E = L;
            return 4;
        case 0x5E:
            E = r_mem(gt_HL());
            return 8;
        case 0x5F:
            E = A;
            return 4;
        case 0x60:
            H = B;
            return 4;
        case 0x61:
            H = C;
            return 4;
        case 0x62:
            H = D;
            return 4;
        case 0x63:
            H = E;
            return 4;
        case 0x64:
            H = H;
            return 4;
        case 0x65:
            H = L;
            return 4;
        case 0x66:
            H = r_mem(gt_HL());
            return 8;
        case 0x67:
            H = A;
            return 4;
        case 0x68:
            L = B;
            return 4;
        case 0x69:
            L = C;
            return 4;
        case 0x6A:
            L = D;
            return 4;
        case 0x6B:
            L = E;
            return 4;
        case 0x6C:
            L = H;
            return 4;
        case 0x6D:
            L = L;
            return 4;
        case 0x6E:
            L = r_mem(gt_HL());
            return 8;
        case 0x6F:
            L = A;
            return 4;
        case 0x70:
            w_mem(gt_HL(), B);
            return 8;
        case 0x71:
            w_mem(gt_HL(), C);
            return 8;
        case 0x72:
            w_mem(gt_HL(), D);
            return 8;
        case 0x73:
            w_mem(gt_HL(), E);
            return 8;
        case 0x74:
            w_mem(gt_HL(), H);
            return 8;
        case 0x75:
            w_mem(gt_HL(), L);
            return 8;
        case 0x76:
            HALT = 1;
            return 4;
        case 0x77:
            w_mem(gt_HL(), A);
            return 8;
        case 0x78:
            A = B;
            return 4;
        case 0x79:
            A = C;
            return 4;
        case 0x7A:
            A = D;
            return 4;
        case 0x7B:
            A = E;
            return 4;
        case 0x7C:
            A = H;
            return 4;
        case 0x7D:
            A = L;
            return 4;
        case 0x7E:
            A = r_mem(gt_HL());
            return 8;
        case 0x7F:
            A = A;
            return 4;
        case 0x80:
            return c_add(B);
        case 0x81:
            return c_add(C);
        case 0x82:
            return c_add(D);
        case 0x83:
            return c_add(E);
        case 0x84:
            return c_add(H);
        case 0x85:
            return c_add(L);
        case 0x86:
            c_add(r_mem(gt_HL()));
            return 8;
        case 0x87:
            return c_add(A);
        case 0x88:
            return c_adc(B);
        case 0x89:
            return c_adc(C);
        case 0x8A:
            return c_adc(D);
        case 0x8B:
            return c_adc(E);
        case 0x8C:
            return c_adc(H);
        case 0x8D:
            return c_adc(L);
        case 0x8E:
            c_adc(r_mem(gt_HL()));
            return 8;
        case 0x8F:
            return c_adc(A);
        case 0x90:
            return c_sub(B);
        case 0x91:
            return c_sub(C);
        case 0x92:
            return c_sub(D);
        case 0x93:
            return c_sub(E);
        case 0x94:
            return c_sub(H);
        case 0x95:
            return c_sub(L);
        case 0x96:
            c_sub(r_mem(gt_HL()));
            return 8;
        case 0x97:
            return c_sub(A);
        case 0x98:
            return c_sbc(B);
        case 0x99:
            return c_sbc(C);
        case 0x9A:
            return c_sbc(D);
        case 0x9B:
            return c_sbc(E);
        case 0x9C:
            return c_sbc(H);
        case 0x9D:
            return c_sbc(L);
        case 0x9E:
            c_sbc(r_mem(gt_HL()));
            return 8;
        case 0x9F:
            return c_sbc(A);
        case 0xA0:
            return c_and(B);
        case 0xA1:
            return c_and(C);
        case 0xA2:
            return c_and(D);
        case 0xA3:
            return c_and(E);
        case 0xA4:
            return c_and(H);
        case 0xA5:
            return c_and(L);
        case 0xA6:
            c_and(r_mem(gt_HL()));
            return 8;
        case 0xA7:
            return c_and(A);
        case 0xA8:
            return c_xor(B);
        case 0xA9:
            return c_xor(C);
        case 0xAA:
            return c_xor(D);
        case 0xAB:
            return c_xor(E);
        case 0xAC:
            return c_xor(H);
        case 0xAD:
            return c_xor(L);
        case 0xAE:
            c_xor(r_mem(gt_HL()));
            return 8;
        case 0xAF:
            return c_xor(A);
        case 0xB0:
            return c_or(B);
        case 0xB1:
            return c_or(C);
        case 0xB2:
            return c_or(D);
        case 0xB3:
            return c_or(E);
        case 0xB4:
            return c_or(H);
        case 0xB5:
            return c_or(L);
        case 0xB6:
            c_or(r_mem(gt_HL()));
            return 8;
        case 0xB7:
            return c_or(A);
        case 0xB8:
            return c_cp(B);
        case 0xB9:
            return c_cp(C);
        case 0xBA:
            return c_cp(D);
        case 0xBB:
            return c_cp(E);
        case 0xBC:
            return c_cp(H);
        case 0xBD:
            return c_cp(L);
        case 0xBE:
            c_cp(r_mem(gt_HL()));
            return 8;
        case 0xBF:
            return c_cp(A);
        case 0xC0:
            return c_ret(1 - gt_flg(FLG_Z));
        case 0xC1:
            st_BC(pop16());
            return 12;
        case 0xC2:
            return c_jp16(1 - gt_flg(FLG_Z));
        case 0xC3:
            return c_jp16(1);
        case 0xC4:
            return c_call(1 - gt_flg(FLG_Z));
        case 0xC5:
            psh16(gt_BC());
            return 16;
        case 0xC6:
            c_add(rd8());
            return 8;
        case 0xC7:
            return c_rst(0x0000);
        case 0xC8:
            return c_ret(gt_flg(FLG_Z));
        case 0xC9:
            c_ret(1);
            return 16;
        case 0xCA:
            return c_jp16(gt_flg(FLG_Z));
        case 0xCC:
            return c_call(gt_flg(FLG_Z));
        case 0xCD:
            return c_call(1);
        case 0xCE:
            c_adc(rd8());
            return 8;
        case 0xCF:
            return c_rst(0x0008);
        case 0xD0:
            return c_ret(1 - gt_flg(FLG_C));
        case 0xD1:
            st_DE(pop16());
            return 12;
        case 0xD2:
            return c_jp16(1 - gt_flg(FLG_C));
        case 0xD4:
            return c_call(1 - gt_flg(FLG_C));
        case 0xD5:
            psh16(gt_DE());
            return 16;
        case 0xD6:
            c_sub(rd8());
            return 8;
        case 0xD7:
            return c_rst(0x0010);
        case 0xD8:
            return c_ret(gt_flg(FLG_C));
        case 0xD9:
            IME = 1;
            c_ret(1);
            return 16;
        case 0xDA:
            return c_jp16(gt_flg(FLG_C));
        case 0xDC:
            return c_call(gt_flg(FLG_C));
        case 0xDE:
            c_sbc(rd8());
            return 8;
        case 0xDF:
            return c_rst(0x0018);
        case 0xE0:
            w_mem(0xFF00 + (dbyte)rd8(), A);
            return 12;
        case 0xE1:
            st_HL(pop16());
            return 12;
        case 0xE2:
            w_mem(0xFF00 + C, A);
            return 8;
        case 0xE5:
            psh16(gt_HL());
            return 16;
        case 0xE6:
            c_and(rd8());
            return 8;
        case 0xE7:
            return c_rst(0x0020);
        case 0xE8:
        {
            byte add = rd8();
            st_h_add((byte)SP, add);
            st_c_add((byte)SP, add);
            SP += (signed char)add;
            cl_flg(FLG_Z);
            cl_flg(FLG_N);
            return 16;
        }
        case 0xE9:
            PC = gt_HL();
            kp();
            return 4;
        case 0xEA:
            w_mem(rd16(), A);
            return 16;
        case 0xEE:
            c_xor(rd8());
            return 8;
        case 0xEF:
            return c_rst(0x0028);
        case 0xF0:
            A = r_mem(0xFF00 + (dbyte)rd8());
            return 12;
        case 0xF1:
            st_AF(pop16() & 0xFFF0);
            return 12;
        case 0xF2:
            A = r_mem(0xFF00 + C);
            return 8;
        case 0xF3:
            IME = 0;
            return 4;
        case 0xF5:
            psh16(gt_AF());
            return 16;
        case 0xF6:
            c_or(rd8());
            return 8;
        case 0xF7:
            return c_rst(0x0030);
        case 0xF8:
        {
            byte nxt = rd8();
            dbyte add = SP + (signed char)nxt;
            st_HL(add);
            cl_flg(FLG_Z);
            cl_flg(FLG_N);
            st_h_add((byte)SP, nxt);
            st_c_add((byte)SP, nxt);
            return 12;
        }
        case 0xF9:
            SP = gt_HL();
            return 8;
        case 0xFA:
            A = r_mem(rd16());
            return 16;
        case 0xFB:
            IME = 1;
            return 4;
        case 0xFE:
            c_cp(rd8());
            return 8;
        case 0xFF:
            return c_rst(0x0038);
        default:
            printf("UNIMPLEMENTED INSTRUCTION\n");
            print_instr(instr, 0);
            return -1;
        }
    }
    return 0;
}
