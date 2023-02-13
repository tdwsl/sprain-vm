#include "sprvm.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **args) {
    FILE *fp;
    uint32_t i;

    if(argc != 2) {
        printf("usage: %s <file>\n", args[0]);
        return 0;
    }

    fp = fopen(args[1], "rb");
    if(!fp) {
        printf("failed to open file\n");
        return 0;
    }

    fread(&r_pc, 4, 1, fp);
    r_sp = 0;
    fread(memory+r_pc, 1, MEMORY_SIZE-r_pc, fp);
    fclose(fp);

    while(i = run()) {
        switch(i) {
        case 1: printf("%d ", r_regs[0]); break;
        }
    }

    return 0;
}
