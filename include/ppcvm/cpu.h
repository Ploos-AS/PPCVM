#ifndef PPCVM_CPU_H
#define PPCVM_CPU_H
#include <stdint.h>
#include "ppcvm/memory.h"
#include "ppcvm/bus.h"
typedef struct {
  uint32_t gpr[32];
  uint32_t pc;
  uint32_t lr;
  uint32_t ctr;
  uint32_t cr;
  uint32_t xer;
} ppcvm_cpu;
typedef enum { PPCVM_OK = 0, PPCVM_UNSUPPORTED = 1, PPCVM_MEMORY_FAULT = 2 } ppcvm_result;
void ppcvm_cpu_reset(ppcvm_cpu *cpu);
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t instruction);
ppcvm_result ppcvm_cpu_step_memory(ppcvm_cpu *cpu, ppcvm_memory *memory, uint32_t instruction);
ppcvm_result ppcvm_cpu_step_bus(ppcvm_cpu *cpu, ppcvm_bus *bus, uint32_t instruction);
#endif
