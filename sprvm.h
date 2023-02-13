#ifndef SPRVM_H
#define SPRVM_H

#include <stdint.h>

#define MEMORY_SIZE 16777216

extern unsigned char memory[MEMORY_SIZE];

extern uint32_t r_pc, r_sp;
extern uint32_t r_regs[4];
extern unsigned char r_sr;

uint32_t getm(uint32_t addr);
uint32_t setm(uint32_t addr, uint32_t v);

uint32_t run();

#endif
