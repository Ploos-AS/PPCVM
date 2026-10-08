#include "ppcvm/cpu.h"
ppcvm_result ppcvm_cpu_step_bus_fetch(ppcvm_cpu *cpu,ppcvm_bus *bus) {
  uint32_t instruction=0;
  if(!cpu || !bus || (cpu->pc&3u)) return PPCVM_MEMORY_FAULT;
  if(ppcvm_bus_read32be(bus,cpu->pc,&instruction)!=PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  return ppcvm_cpu_step_bus(cpu,bus,instruction);
}
