#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bus bus;
  ppcvm_cpu cpu;
  uint8_t ram[12]={0};
  ppcvm_bus_init(&bus);
  ppcvm_cpu_reset(&cpu);
  assert(ppcvm_bus_map_memory(&bus,0,sizeof(ram),ram,0)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,0,(14u<<26)|(3u<<21)|7u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,4,PPCVM_DIAGNOSTIC_HALT)==PPCVM_BUS_OK);
  ppcvm_run_report r=ppcvm_cpu_run_bus_diagnostic(&cpu,&bus,4);
  assert(r.reason==PPCVM_RUN_HALT && r.executed==1 && r.final_pc==4);
  assert(cpu.gpr[3]==7 && cpu.pc==4);
  r=ppcvm_cpu_run_bus_diagnostic(&cpu,&bus,1);
  assert(r.reason==PPCVM_RUN_HALT && r.executed==0 && r.final_pc==4);
  r=ppcvm_cpu_run_bus(&cpu,&bus,1);
  assert(r.reason==PPCVM_RUN_UNSUPPORTED && r.executed==0 && r.final_pc==4);
  ppcvm_cpu_reset(&cpu);
  r=ppcvm_cpu_run_bus_diagnostic(&cpu,&bus,1);
  assert(r.reason==PPCVM_RUN_LIMIT && r.executed==1 && r.final_pc==4);
  r=ppcvm_cpu_run_bus_diagnostic(0,&bus,1);
  assert(r.reason==PPCVM_RUN_INVALID);
  cpu.pc=12;
  r=ppcvm_cpu_run_bus_diagnostic(&cpu,&bus,1);
  assert(r.reason==PPCVM_RUN_MEMORY_FAULT && r.executed==0 && r.final_pc==12);
  return 0;
}
