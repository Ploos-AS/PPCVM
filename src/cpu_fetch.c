#include "ppcvm/cpu.h"
ppcvm_result ppcvm_cpu_step_bus_fetch(ppcvm_cpu *cpu,ppcvm_bus *bus) {
  uint32_t instruction=0;
  if(!cpu || !bus || (cpu->pc&3u)) return PPCVM_MEMORY_FAULT;
  if(ppcvm_bus_read32be(bus,cpu->pc,&instruction)!=PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  return ppcvm_cpu_step_bus(cpu,bus,instruction);
}

ppcvm_run_report ppcvm_cpu_run_bus(ppcvm_cpu *cpu,ppcvm_bus *bus,
                                    uint64_t max_steps) {
  ppcvm_run_report report={PPCVM_RUN_INVALID,0,cpu ? cpu->pc : 0};
  if(!cpu || !bus) return report;
  report.reason=PPCVM_RUN_LIMIT;
  for(uint64_t i=0;i<max_steps;i++) {
    ppcvm_result result=ppcvm_cpu_step_bus_fetch(cpu,bus);
    if(result!=PPCVM_OK) {
      report.reason=result==PPCVM_UNSUPPORTED ?
        PPCVM_RUN_UNSUPPORTED : PPCVM_RUN_MEMORY_FAULT;
      break;
    }
    report.executed++;
  }
  report.final_pc=cpu->pc;
  return report;
}
