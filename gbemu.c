#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "var.h"
#include "func.h"
#include "disassembler.h"
#include "cpu.h"
#include "dsp.h"

int update() {
    scn = 456;
    frame = 0;
    dma = 0;
    int d = 0;

    while (!frame) {
        d = 0;
        switch (PC) { // PC
        case 0xB702:
            d = 1;
        }
        if (HALT) {
            if (chck_intr()) HALT = 0;
            scn -= 4;
        }
        else {
            int curr = exec();
            if (d) {
                dbg();
                SDL_Delay(5000);
            }
            if (curr == -1) return 1;
            OP_CNT++;
            PC++;
            scn -= curr;
        }
        if (scn <= 0) {
            scnln();
            scn = 456;
        }
        upd_stat();
        chck_in();
        chck_dma();
        upd_tim();
        chck_intr();
    }
    return 0;
}

int main(int argc, char* argv[]) {
    init_reg();
    init_dsp();
    FILE* boot_rom;
    FILE* rom;
    fopen_s(&boot_rom, "bootrom.rom", "rb");
    fopen_s(&rom, "space.gb", "rb");
    fread(mem, 0x8000, 1, rom);
    fread(brom, 0x100, 1, boot_rom);
    while (1) {
        if (update()) break;
        if (rndr()) break;
    }
    dbg();
    printf("DONE %d\n", OP_CNT);
    return 0;
}
