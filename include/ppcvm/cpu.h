#ifndef PPCVM_CPU_H
#define PPCVM_CPU_H
#include <stdint.h>
typedef struct {
  uint32_t gpr[32];
  uint32_t pc;
  uint32_t lr;
  uint32_t ctr;
  uint32_t cr;
  uint32_t xer;
} ppcvm_cpu;
typedef enum { PPCVM_OK = 0, PPCVM_UNSUPPORTED = 1 } ppcvm_result;
void ppcvm_cpu_reset(ppcvm_cpu *cpu);
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t instruction);
#endif
