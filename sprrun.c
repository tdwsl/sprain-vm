#include "sprvm.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char **args) {
    FILE *fp;
    unsigned char i;

    if(argc < 2) {
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

    while(i = run()) { // int 0 = quit
        switch(i) {
        case 1:
            printf("%d ", r_regs[1]);
            break;
        case 0x02: // PrintChar
            printf("%c", r_regs[1]&0xff);
            break;
        case 0x08: // Argc
            r_regs[1] = argc-1;
            break;
        case 0x09: // Argv
            if(r_regs[1]+1 >= argc) {
                r_regs[1] = 0xffffffff;
                break;
            }
            if(strlen(args[r_regs[1]+1]) >= r_regs[2]) {
                r_regs[1] = 0xffffffff;
            } else {
                strcpy((char*)&memory[r_regs[3]], args[r_regs[1]+1]);
                r_regs[1] = 0;
            }
            r_regs[2] = strlen(args[r_regs[1]]);
            break;
        case 0x10: // FOpen
            if(r_regs[1] == 0)
                fp = fopen((char*)&memory[r_regs[2]], "rb");
            else if(r_regs[1] == 1)
                fp = fopen((char*)&memory[r_regs[2]], "wb");
            else { r_regs[1] = 0xffffffff; break; }
            if(fp) r_regs[1] = 0;
            else r_regs[1] = 0xffffffff;
            break;
        case 0x11: // FClose
            fclose(fp);
            break;
        case 0x12: // FWriteChar
            fprintf(fp, "%c", r_regs[1]);
            break;
        case 0x13: // FWrite
            fwrite(memory+r_regs[2], r_regs[1], 1, fp);
            break;
        case 0x14: // FReadChar
            r_regs[1] = fgetc(fp);
            break;
        case 0x15: // FRead
            r_regs[1] = fread(memory+r_regs[2], r_regs[1], 1, fp);
            break;
        case 0x16: // FEOF
            if(feof(fp)) r_regs[1] = 0xffffffff;
            else r_regs[1] = 0;
            break;
        case 0x69: // ToggleDebug
            debug = !debug;
            break;
        }
    }

    return 0;
}
