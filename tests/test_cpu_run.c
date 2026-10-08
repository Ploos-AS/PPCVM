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
  assert(ppcvm_bus_write32be(&bus,0,(14u<<26)|(3u<<21)|1u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,4,(14u<<26)|(3u<<21)|(3u<<16)|2u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,8,(14u<<26)|(3u<<21)|(3u<<16)|3u)==PPCVM_BUS_OK);
  ppcvm_run_report r=ppcvm_cpu_run_bus(&cpu,&bus,0);
  assert(r.reason==PPCVM_RUN_LIMIT && r.executed==0 && r.final_pc==0);
  r=ppcvm_cpu_run_bus(&cpu,&bus,2);
  assert(r.reason==PPCVM_RUN_LIMIT && r.executed==2 && r.final_pc==8);
  assert(cpu.gpr[3]==3);
  r=ppcvm_cpu_run_bus(&cpu,&bus,3);
  assert(r.reason==PPCVM_RUN_MEMORY_FAULT && r.executed==1 && r.final_pc==12);
  assert(cpu.gpr[3]==6);
  r=ppcvm_cpu_run_bus(&cpu,&bus,1);
  assert(r.reason==PPCVM_RUN_MEMORY_FAULT && r.executed==0 && r.final_pc==12);
  r=ppcvm_cpu_run_bus(0,&bus,1);
  assert(r.reason==PPCVM_RUN_INVALID && r.executed==0);
  r=ppcvm_cpu_run_bus(&cpu,0,1);
  assert(r.reason==PPCVM_RUN_INVALID && r.executed==0);
  return 0;
}
