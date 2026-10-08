#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define LWZ ((32u<<26)|(3u<<21)|(4u<<16))
#define STW ((36u<<26)|(3u<<21)|(4u<<16))
int main(void) {
  ppcvm_bus bus;
  ppcvm_bus_init(&bus);
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  cpu.pc=0x100; cpu.gpr[4]=0x2000; cpu.msr=UINT32_C(0x0000c030);
  assert(ppcvm_cpu_step_bus(&cpu,&bus,LWZ)==PPCVM_MEMORY_FAULT);
  assert(cpu.pc==0x100 && cpu.dar==0);
  assert(ppcvm_cpu_step_bus_dsi(&cpu,&bus,LWZ)==PPCVM_OK);
  assert(cpu.pc==PPCVM_VECTOR_DSI && cpu.srr0==0x100);
  assert(cpu.dar==0x2000 && cpu.dsisr==UINT32_C(0x40000000));
  assert(cpu.srr1==UINT32_C(0x0000c030) && cpu.msr==0);
  ppcvm_cpu_reset(&cpu); cpu.pc=0x200; cpu.gpr[4]=0x3000;
  assert(ppcvm_cpu_step_bus_dsi(&cpu,&bus,STW)==PPCVM_OK);
  assert(cpu.pc==PPCVM_VECTOR_DSI && cpu.dar==0x3000);
  assert(cpu.dsisr==UINT32_C(0x42000000));
  ppcvm_cpu_reset(&cpu); cpu.pc=0x200; cpu.gpr[4]=0x3001;
  assert(ppcvm_cpu_step_bus_dsi(&cpu,&bus,LWZ)==PPCVM_MEMORY_FAULT);
  assert(cpu.pc==0x200 && cpu.dar==0);
  return 0;
}
