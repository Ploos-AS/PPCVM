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
  uint32_t msr;
  uint32_t srr0;
  uint32_t srr1;
  uint32_t dar;
  uint32_t dsisr;
} ppcvm_cpu;
typedef enum { PPCVM_OK = 0, PPCVM_UNSUPPORTED = 1, PPCVM_MEMORY_FAULT = 2 } ppcvm_result;
/* Minimal low-vector exception entry. Caller supplies architected resume PC. */
#define PPCVM_VECTOR_ISI UINT32_C(0x400)
#define PPCVM_VECTOR_DSI UINT32_C(0x300)
#define PPCVM_VECTOR_PROGRAM UINT32_C(0x700)
#define PPCVM_VECTOR_SYSCALL UINT32_C(0xc00)
void ppcvm_cpu_enter_exception(ppcvm_cpu *cpu, uint32_t vector, uint32_t resume_pc);
void ppcvm_cpu_reset(ppcvm_cpu *cpu);
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t instruction);
ppcvm_result ppcvm_cpu_step_memory(ppcvm_cpu *cpu, ppcvm_memory *memory, uint32_t instruction);
ppcvm_result ppcvm_cpu_step_bus(ppcvm_cpu *cpu, ppcvm_bus *bus, uint32_t instruction);
ppcvm_result ppcvm_cpu_step_bus_dsi(ppcvm_cpu *cpu, ppcvm_bus *bus, uint32_t instruction);
/* Fetch one big-endian instruction from the bus at PC, then execute it.
   No MMU translation or instruction-side exception delivery. */
ppcvm_result ppcvm_cpu_step_bus_fetch(ppcvm_cpu *cpu, ppcvm_bus *bus);
typedef enum {
  PPCVM_RUN_LIMIT=0,
  PPCVM_RUN_UNSUPPORTED=1,
  PPCVM_RUN_MEMORY_FAULT=2,
  PPCVM_RUN_INVALID=3,
  PPCVM_RUN_HALT=4
} ppcvm_run_reason;
typedef struct {
  ppcvm_run_reason reason;
  uint64_t executed;
  uint32_t final_pc;
} ppcvm_run_report;
/* Execute at most max_steps instructions; zero is a valid no-op limit. */
ppcvm_run_report ppcvm_cpu_run_bus(ppcvm_cpu *cpu, ppcvm_bus *bus,
                                    uint64_t max_steps);
/* PPCVM-only opt-in diagnostic stop word, not a PowerPC HALT opcode.
   HALT leaves PC pointing at the marker and is not counted as executed. */
#define PPCVM_DIAGNOSTIC_HALT UINT32_C(0x00000000)
ppcvm_run_report ppcvm_cpu_run_bus_diagnostic(ppcvm_cpu *cpu,
    ppcvm_bus *bus, uint64_t max_steps);
#endif
