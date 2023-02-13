#include "sprvm.h"
#include <stdint.h>

#define MEMORY_SIZE 16777216

unsigned char memory[MEMORY_SIZE];

uint32_t r_pc, r_sp;
uint32_t r_regs[4];
unsigned char r_sr;

uint32_t getm(uint32_t addr) {
    return *(uint32_t*)&memory[addr];
}

void setm(uint32_t addr, uint32_t v) {
    *(uint32_t*)&memory[addr] = v;
}

void setb(uint32_t *v, unsigned char b) {
    *v ^= 0xff;
    *v |= b;
}

void reljmp() {
    r_pc += 1 + memory[r_pc];
}

void push(uint32_t v) {
    setm(r_sp, v);
    r_sp += 4;
}

uint32_t pop() {
    r_sp -= 4;
    return getm(r_sp);
}

uint32_t *reg1(unsigned char ins) {
    return &r_regs[(ins&0x0f)/4];
}

void flags(uint32_t v) {
    r_sr = 0;
    if(v == 0) r_sr |= 1;
    if(v & 0x80000000) r_sr |= 4;
}

void bflags(unsigned char v) {
    r_sr = 0;
    if(v == 0) r_sr |= 1;
    if(v & 0x80) r_sr |= 4;
}

void inc(uint32_t *v) {
    *v += 1;
    flags(*v);
    if(*v == 0) r_sr |= 2;
}

void dec(uint32_t *v) {
    *v -= 1;
    flags(*v);
    if(*v == 0xffffffff) r_sr |= 2;
}

void incb(uint32_t *v) {
    setb(v, *v+1);
    bflags(*v);
    if((*v)&0xff == 0) r_sr |= 2;
}

void decb(uint32_t *v) {
    setb(v, *v-1);
    bflags(*v);
    if((*v)&0xff == 0xff) r_sr |= 2;
}

void shr(uint32_t *v, unsigned char s) {
    unsigned char l;
    l = *v & (1<<(s-1));
    *v >>= s;
    flags(*v);
    if(l) r_sr |= 2;
}

void shl(uint32_t *v, unsigned char s) {
    unsigned char l;
    l = *v & (0x80000000>>(s-1));
    *v <<= s;
    flags(*v);
    if(l) r_sr |= 2;
}

void add(uint32_t *v, uint32_t a) {
    uint32_t o;
    o = *v;
    *v += a;
    flags(*v);
    if(*v < o)
        r_sr |= 4;
}

void addb(uint32_t *v, unsigned char a) {
    unsigned char o, b;
    b = *v;
    o = b;
    b += a;
    bflags(b);
    if(b < o)
        r_sr |= 4;
    *v ^= 0xff;
    *v |= b;
}

uint32_t run() {
    unsigned char ins;
    uint32_t v;
    for(;;) {
        ins = memory[r_pc++];
        switch(ins & 0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                r_pc--;
                break;
            case 0x01:
                return memory[r_pc++];
            case 0x02:
                r_pc = getm(r_pc);
                break;
            case 0x03:
                reljmp();
                break;
            case 0x04:
                push(r_pc+4);
                r_pc = getm(r_pc);
                break;
            case 0x05:
                r_pc = pop();
                break;
            case 0x06:
                r_regs[0] = r_sp;
                break;
            case 0x07:
                r_sp = r_regs[0];
                break;
            case 0x08:
                setb(&regs[0], r_sr);
                break;
            case 0x09:
                r_sr = regs[0];
                break;
            case 0x0A:
                r_sr |= 2;
                break;
            case 0x0B:
                r_sr ^= 2;
                break;
            }
            break;
        case 0x10:
            switch((ins&0x0f)/4) {
            case 0:
                r_regs[ins&0x03] = getm(r_pc);
                r_pc += 4;
                break;
            case 1:
                setb(&r_regs[ins&0x03], memory[r_pc]);
                r_pc++;
                break;
            case 2:
                r_regs[ins&0x03] = getm(getm(r_pc));
                r_pc += 4;
                break;
            case 3:
                setb(&r_regs[ins&0x03], memory[getm(r_pc)]);
                r_pc += 4;
                break;
            }
            break;
        case 0x20:
            *reg1(ins) = getm(r_regs[ins&0x03]);
            break;
        case 0x30:
            setb(reg1(ins), memory[r_regs[ins&0x03]]);
            break;
        case 0x40:
            setm(r_regs[ins&0x03], *reg1(ins));
            break;
        case 0x50:
            memory[r_regs[ins&0x03]] = *reg1(ins);
            break;
        case 0x60:
            switch((ins&0x0f)/4) {
            case 0:
                inc(&r_regs[ins&0x03]);
                break;
            case 1:
                incb(&r_regs[ins&0x03]);
                break;
            case 2:
                dec(&r_regs[ins&0x03]);
                break;
            case 3:
                decb(&r_regs[ins&0x03]);
                break;
            }
            break;
        case 0x70:
            switch((ins&0x0f)/4) {
            case 0:
                regs[ins&0x03] = ~regs[ins&0x03];
                flags(regs[ins&0x03]);
                break;
            case 1:
                setb(&regs[ins&0x03], ~regs[ins&0x03]);
                bflags(regs[ins&0x03]);
                break;
            case 2:
                regs[ins&0x03] &= getm(r_pc);
                flags(regs[ins&0x03]);
                r_pc += 4;
                break;
            case 3:
                setb(&regs[ins&0x03], regs[ins&0x03] & memory[r_pc]);
                bflags(regs[ins&0x03]);
                r_pc++;
                break;
            }
            break;
        case 0x80:
            switch((ins&0x0f)/4) {
            case 0:
                regs[ins&0x03] |= getm(r_pc);
                flags(regs[ins&0x03]);
                r_pc += 4;
                break;
            case 1:
                setb(regs[ins&0x03], regs[ins&0x03] | memory[r_pc]);
                bflags(regs[ins&0x03]);
                r_pc++;
                break;
            case 2:
                regs[ins&0x03] ^= getm(r_pc);
                flags(regs[ins&0x03]);
                r_pc += 4;
                break;
            case 3:
                setb(regs[ins&0x03], regs[ins&0x03] | memory[r_pc]);
                bflags(regs[ins&0x03]);
                r_pc++;
                break;
            }
            break;
        case 0x90:
            switch((ins[0x0f])/4) {
            case 0:
                shr(&regs[ins&0x03], memory[r_pc]);
                r_pc++;
                break;
            case 1:
                shl(&regs[ins&0x03], memory[r_pc]);
                r_pc++;
                break;
            case 2:
                add(&regs[ins&0x03], getm(r_pc));
                r_pc += 4;
                break;
            case 3:
                addb(&regs[ins&0x03], memory[r_pc]);
                r_pc++;
                break;
            }
            break;
        case 0xA0:
            switch((ins[0x0f])/4) {
            case 0:
                v = regs[ins&0x03];
                add(&v, ~getm(r_pc)+1);
                r_pc += 4;
                break;
            case 1:
                v = regs[ins&0x03];
                add(&v, ~memory[r_pc]+1);
                r_pc++;
                break;
            case 2:
                push(regs[ins&0x03]);
                break;
            case 3:
                regs[ins&0x03] = pop();
                break;
            }
            break;
        case 0xB0:
            switch((regs[ins&0x0f])/4) {
            case 0:
                r_pc = regs[ins&0x03];
                break;
            case 1:
                push(r_pc);
                r_pc = regs[ins&0x03];
                break;
            case 2:
                if(r_flags & (1<<(ins&0x03)))
                    reljmp();
                break;
            case 3:
                if(r_flags & (1<<(ins&0x03)) == 0)
                    reljmp();
                break;
            }
            break;
        default:
            if(ins == 0xFF) {
                ins = memory[r_pc++];
                switch(ins&0xf0) {
                case 0x00:
                    *reg1(ins) &= r_regs[ins&0x03];
                    flags(*reg1(ins));
                    break;
                case 0x01:
                    setb(reg1(ins), *reg1(ins)&r_regs[ins&0x03]);
                    bflags(*reg1(ins));
                    break;
                case 0x02:
                    *reg1(ins) |= r_regs[ins&0x03];
                    flags(*reg1(ins));
                    break;
                case 0x03:
                    setb(reg1(ins), *reg1(ins)|r_regs[ins&0x03]);
                    bflags(*reg1(ins));
                    break;
                case 0x04:
                    *reg1(ins) ^= r_regs[ins&0x03];
                    flags(*reg1(ins));
                    break;
                case 0x05:
                    setb(reg1(ins), *reg1(ins)^r_regs[ins&0x03]);
                    bflags(*reg1(ins));
                    break;
                case 0x06:
                    add(reg1(ins), r_regs[ins&0x03]);
                    break;
                case 0x07:
                    addb(reg1(ins), r_regs[ins&0x03]);
                    break;
                case 0x08:
                    add(reg1(ins), ~r_regs[ins&0x03]+1);
                    break;
                case 0x09:
                    addb(reg1(ins), ~r_regs[ins&0x03]+1);
                    break;
                case 0x0A:
                    v = *reg1(ins);
                    add(&v, ~r_regs[ins&0x03]+1);
                    break;
                case 0x0B:
                    v = *reg1(ins);
                    addb(&v, ~r_regs[ins&0x03]+1);
                    break;
                }
            }
        }
    }
}

