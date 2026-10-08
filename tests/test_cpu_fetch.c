#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bus bus;
  ppcvm_cpu cpu;
  uint8_t ram[16]={0};
  ppcvm_bus_init(&bus);
  ppcvm_cpu_reset(&cpu);
  assert(ppcvm_bus_map_memory(&bus,0,sizeof(ram),ram,0)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,0,(14u<<26)|(3u<<21)|7u)==PPCVM_BUS_OK);
  assert(ppcvm_cpu_step_bus_fetch(&cpu,&bus)==PPCVM_OK);
  assert(cpu.gpr[3]==7 && cpu.pc==4);
  cpu.pc=2;
  assert(ppcvm_cpu_step_bus_fetch(&cpu,&bus)==PPCVM_MEMORY_FAULT);
  assert(cpu.pc==2);
  cpu.pc=16;
  assert(ppcvm_cpu_step_bus_fetch(&cpu,&bus)==PPCVM_MEMORY_FAULT);
  assert(cpu.pc==16);
  assert(ppcvm_cpu_step_bus_fetch(0,&bus)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_cpu_step_bus_fetch(&cpu,0)==PPCVM_MEMORY_FAULT);
  return 0;
}
