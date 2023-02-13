#ifndef SPRVM_H
#define SPRVM_H

#include <stdint.h>

#define MEMORY_SIZE 16777216

extern unsigned char memory[MEMORY_SIZE];
extern uint32_t r_regs[16];
extern unsigned char debug;

uint32_t getm(uint32_t addr);
void setm(uint32_t addr, uint32_t v);

unsigned char run();

#endif
