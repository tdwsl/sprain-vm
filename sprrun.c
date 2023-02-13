#include "sprvm.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **args) {
    FILE *fp;
    unsigned char i;

    if(argc != 2) {
        printf("usage: %s <file>\n", args[0]);
        return 0;
    }

    fp = fopen(args[1], "rb");
    if(!fp) {
        printf("failed to open file\n");
        return 0;
    }

    fread(&r_regs[15], 4, 1, fp);
    r_regs[14] = 0;
    fread(memory+r_regs[15], 1, MEMORY_SIZE-r_regs[15], fp);
    fclose(fp);

    while(i = run()) {
        switch(i) {
        case 1: printf("%d ", r_regs[1]); break;
        case 2: printf("%c", r_regs[1]&0xff); break;
        case 0x69: debug = !debug; break;
        }
    }

    return 0;
}
