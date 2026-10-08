#ifndef PPCVM_PEGASOS2_H
#define PPCVM_PEGASOS2_H
#include "ppcvm/bus.h"
#include "ppcvm/cpu.h"
#include "ppcvm/memory.h"
#include <stddef.h>
#include <stdint.h>
/* Provisional discovery window for testing, not a validated Pegasos II map. */
#define PPCVM_PEGASOS2_DISCOVERY_BASE UINT32_C(0xf1000000)
#define PPCVM_PEGASOS2_DISCOVERY_SIZE UINT32_C(0x1000)
typedef struct {
  ppcvm_cpu cpu;
  ppcvm_memory ram;
  ppcvm_bus bus;
  uint32_t discovery_scratch;
  uint32_t discovery_reads;
  uint32_t discovery_writes;
} ppcvm_pegasos2;
int ppcvm_pegasos2_init(ppcvm_pegasos2 *machine, size_t ram_size);
/* Fetches one big-endian instruction from mapped memory and executes it. */
ppcvm_result ppcvm_pegasos2_step(ppcvm_pegasos2 *machine);
void ppcvm_pegasos2_destroy(ppcvm_pegasos2 *machine);
#endif
