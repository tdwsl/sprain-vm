#include "sprvm.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef CONIO
#include <conio.h>
#else
#include <ncurses.h>
#endif

int main(int argc, char **args) {
    FILE *fp;
    unsigned char i;
    clock_t start;

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

    start = clock();

#ifdef CONIO
    clrscr();
#else
    initscr();
    keypad(stdscr, 1);
    nodelay(stdscr, 1);
#endif

    while(i = run()) { // int 0 = quit
        switch(i) {
#ifdef CONIO
        case 0x20: // PutChar
            putch(r_regs[1]);
            break;
        case 0x21: // GetChar
            r_regs[1] = getch();
            break;
        case 0x22: // KeyDown
            if(kbhit()) r_regs[1] = 0xffffffff;
            else r_regs[1] = 0;
            break;
        case 0x24: // Cursor
            gotoxy(r_regs[3]+1, r_regs[2]+1);
            break;
        case 0x25: // Window
            r_regs[3] = 80;
            r_regs[2] = 24;
            break;
        case 0x26: // Clear
            clrscr();
            break;
#else
        case 0x20: // PutChar
            addch(r_regs[1]);
            break;
        case 0x21: // GetChar
            while((r_regs[1] = getch()) == ERR);
            switch(r_regs[1]) {
            case KEY_UP: r_regs[1] = 165; break;
            case KEY_DOWN: r_regs[1] = 166; break;
            case KEY_RIGHT: r_regs[1] = 167; break;
            case KEY_LEFT: r_regs[1] = 168; break;
            }
            break;
        case 0x22: // KeyDown
            if((r_regs[1] = getch()) == ERR)
                r_regs[1] = 0;
            else {
                ungetch(r_regs[1]);
                r_regs[1] = 0xffffffff;
            }
            break;
        case 0x24: // Cursor
            move(r_regs[2], r_regs[3]);
            break;
        case 0x25: // Window
            getmaxyx(stdscr, r_regs[2], r_regs[3]);
            break;
        case 0x26: // Clear
            clear();
            break;
#endif
        case 0x30: // Ticks
            r_regs[1] = (clock()-start)/((double)CLOCKS_PER_SEC/1000.0);
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

#ifdef CONIO
    clrscr();
#else
    nodelay(stdscr, 0);
    keypad(stdscr, 0);
    endwin();
#endif

    return 0;
}
