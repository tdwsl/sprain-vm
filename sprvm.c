#include "sprvm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

uint8_t memory[MEMORY_SIZE];
uint32_t r_regs[16];
bool debug = 0;
FILE *logfp;

uint32_t getm(uint32_t addr) {
    return *(uint32_t*)&memory[addr];
}

void setm(uint32_t addr, uint32_t v) {
    *(uint32_t*)&memory[addr] = v;
}

void printIns(uint32_t pc) {
    const char *insStrs[] = {
        0, "MOV", "ADD", "PUSH", "POP", "INV", "SHR", "SHL",
    };
    const char *linsStrs[] = {
        "INT", "CALL", "BEQ", "BNE", "BGE", "BLT", "MOV", "MOV",
        "MOV", "MOV", "MOV", "AND", "OR", "XOR", "ADD", "SUB",
    };
    const char *regStrs[] = {
        "ZERO", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "R13", "RSP", "RPC",
    };
    const char *lregStrs[] = {
        "LZERO", "L1", "L2", "L3", "L4", "L5", "L6", "L7",
        "L8", "L9", "L10", "L11", "L12", "L13", "LSP", "LPC",
    };
    const char *nregStrs[] = {
        "(ZERO)", "(R1)", "(R2)", "(R3)", "(R4)", "(R5)", "(R6)", "(R7)",
        "(R8)", "(R9)", "(R10)", "(R11)", "(R12)", "(R13)", "(RSP)", "(RPC)",
    };
    fprintf(logfp, "%.8X ", pc);
    if(memory[pc]&0xf0) fprintf(logfp, "%s ", insStrs[memory[pc]>>4]);
    else fprintf(logfp, "%s ", linsStrs[memory[pc]]);
    switch(memory[pc]&0xf0) {
    case 0x00:
        switch(memory[pc]) {
        case 0x00:
            fprintf(logfp, "%.2X", memory[pc+1]);
            break;
        case 0x01:
            fprintf(logfp, "%.4X", getm(pc+1));
            break;
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            fprintf(logfp, "%s,%s %.2X",
              regStrs[memory[pc+1]>>4], regStrs[memory[pc+1]&0x0f],
              memory[pc+2]);
            break;
        case 0x06:
            fprintf(logfp, "%s,%s",
              regStrs[memory[pc+1]>>4], nregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x07:
            fprintf(logfp, "%s,%s",
              nregStrs[memory[pc+1]>>4], regStrs[memory[pc+1]&0x0f]);
            break;
        case 0x08:
            fprintf(logfp, "%s,%s",
              lregStrs[memory[pc+1]>>4], nregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x09:
            fprintf(logfp, "%s,%s",
              nregStrs[memory[pc+1]>>4], lregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
            fprintf(logfp, "%s,%s",
              regStrs[memory[pc+1]>>4], regStrs[memory[pc+1]&0x0f]);
            break;
        }
        break;
    case 0x10:
        fprintf(logfp, "%s,%.8X",
          regStrs[memory[pc]&0x0f], getm(pc+1));
        break;
    case 0x20:
    case 0x60:
    case 0x70:
        fprintf(logfp, "%s,%.2X",
          regStrs[memory[pc]&0x0f], memory[pc+1]);
        break;
    case 0x30:
    case 0x40:
    case 0x50:
        fprintf(logfp, "%s", regStrs[memory[pc]&0x0f]);
        break;
    }
    fprintf(logfp, "\n");
}

uint8_t run() {
    uint8_t ins;
    int i;
    for(;;) {
        ins = memory[r_regs[15]++];
        r_regs[0] = 0;
        if(debug) {
            fprintf(logfp, "         ");
            for(i = 1; i < 15; i++) fprintf(logfp, "%X ", r_regs[i]);
            fprintf(logfp, "\n\n");
            printIns(r_regs[15]-1);
            fprintf(logfp, "\n");
        }
        switch(ins&0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                return memory[r_regs[15]++];
            case 0x01:
                r_regs[15] += 4;
                setm(r_regs[14], r_regs[15]);
                r_regs[14] += 4;
                r_regs[15] = getm(r_regs[15]-4);
                break;
            case 0x02:
                ins = memory[r_regs[15]];
                r_regs[15] += 2;
                if(r_regs[ins>>4] == r_regs[ins&0x0f])
                    r_regs[15] += (int8_t)memory[r_regs[15]-1];
                break;
            case 0x03:
                ins = memory[r_regs[15]];
                r_regs[15] += 2;
                if(r_regs[ins>>4] != r_regs[ins&0x0f])
                    r_regs[15] += (int8_t)memory[r_regs[15]-1];
                break;
            case 0x04:
                ins = memory[r_regs[15]];
                r_regs[15] += 2;
                if(!((r_regs[ins>>4] - r_regs[ins&0x0f]) & 0x80000000))
                    r_regs[15] += (int8_t)memory[r_regs[15]-1];
                break;
            case 0x05:
                ins = memory[r_regs[15]];
                r_regs[15] += 2;
                if((r_regs[ins>>4] - r_regs[ins&0x0f]) & 0x80000000)
                    r_regs[15] += (int8_t)memory[r_regs[15]-1];
                break;
            case 0x06:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] = getm(r_regs[ins&0x0f]);
                break;
            case 0x07:
                ins = memory[r_regs[15]++];
                setm(r_regs[ins>>4], r_regs[ins&0x0f]);
                break;
            case 0x08:
                ins = memory[r_regs[15]++];
                *(uint8_t*)&r_regs[ins>>4] = memory[r_regs[ins&0x0f]];
                break;
            case 0x09:
                ins = memory[r_regs[15]++];
                memory[r_regs[ins>>4]] = r_regs[ins&0x0f];
                break;
            case 0x0a:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] = r_regs[ins&0x0f];
                break;
            case 0x0b:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] &= r_regs[ins&0x0f];
                break;
            case 0x0c:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] |= r_regs[ins&0x0f];
                break;
            case 0x0d:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] ^= r_regs[ins&0x0f];
                break;
            case 0x0e:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] += r_regs[ins&0x0f];
                break;
            case 0x0f:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] -= r_regs[ins&0x0f];
                break;
            }
            break;
        case 0x10:
            r_regs[15] += 4;
            r_regs[ins&0x0f] = getm(r_regs[15]-4);
            break;
        case 0x20:
            r_regs[15]++;
            r_regs[ins&0x0f] += (int8_t)memory[r_regs[15]-1];
            break;
        case 0x30:
            setm(r_regs[14], r_regs[ins&0x0f]);
            r_regs[14] += 4;
            break;
        case 0x40:
            r_regs[14] -= 4;
            r_regs[ins&0x0f] = getm(r_regs[14]);
            break;
        case 0x50:
            r_regs[ins&0x0f] = ~r_regs[ins&0x0f];
            break;
        case 0x60:
            r_regs[15]++;
            r_regs[ins&0x0f] >>= memory[r_regs[15]-1];
            break;
        case 0x70:
            r_regs[15]++;
            r_regs[ins&0x0f] <<= memory[r_regs[15]-1];
            break;
        }
    }
}
