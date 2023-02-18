#ifndef SPRVM_H
#define SPRVM_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 16777216

extern uint8_t memory[MEMORY_SIZE];
extern uint32_t r_regs[16];
extern bool debug;

uint32_t getm(uint32_t addr);
void setm(uint32_t addr, uint32_t v);

uint8_t run();

#endif
