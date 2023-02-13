#include "sprvm.h"
#include <stdint.h>
#include <stdio.h>

unsigned char memory[MEMORY_SIZE];
uint32_t r_regs[16];
unsigned char debug = 0;

uint32_t getm(uint32_t addr) {
    return *(uint32_t*)&memory[addr];
}

void setm(uint32_t addr, uint32_t v) {
    *(uint32_t*)&memory[addr] = v;
}

unsigned char run() {
    unsigned char ins;
    int i;
    for(;;) {
        ins = memory[r_regs[15]++];
        r_regs[0] = 0;
        if(debug) {
            for(i = 0; i < 16; i++) printf("%d ", r_regs[i]);
            printf("\n%.8x %.2x\n", r_regs[15]-1, ins);
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
                    r_regs[15] += (char)memory[r_regs[15]-1];
                break;
            case 0x03:
                ins = memory[r_regs[15]];
                r_regs[15] += 2;
                if(r_regs[ins>>4] != r_regs[ins&0x0f])
                    r_regs[15] += (char)memory[r_regs[15]-1];
                break;
            case 0x04:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] = getm(r_regs[ins&0x0f]);
                break;
            case 0x05:
                ins = memory[r_regs[15]++];
                setm(r_regs[ins>>4], r_regs[ins&0x0f]);
                break;
            case 0x06:
                ins = memory[r_regs[15]++];
                *(unsigned char*)&r_regs[ins>>4] = memory[r_regs[ins&0x0f]];
                break;
            case 0x07:
                    ins = memory[r_regs[15]++];
                memory[r_regs[ins>>4]] = memory[r_regs[ins&0x0f]];
                break;
            case 0x08:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] = r_regs[ins&0x0f];
                break;
            case 0x09:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] &= r_regs[ins&0x0f];
                break;
            case 0x0a:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] |= r_regs[ins&0x0f];
                break;
            case 0x0b:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] ^= r_regs[ins&0x0f];
                break;
            case 0x0c:
                ins = memory[r_regs[15]++];
                r_regs[ins>>4] += r_regs[ins&0x0f];
                break;
            case 0x0d:
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
            r_regs[ins&0x0f] += (char)memory[r_regs[15]-1];
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
