#include "ppcvm/irq_bridge.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_irq_latch latch = {0};
  ppcvm_cpu_reset(&cpu);
  cpu.pc=0x1000u;
  ppcvm_irq_latch_set(&latch,1u);
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==0);
  ppcvm_irq_latch_enable(&latch,1u);
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==0); /* MSR EE off */
  cpu.msr=0x8000u;
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==1);
  assert(cpu.pc==PPCVM_VECTOR_EXTERNAL && cpu.srr0==0x1000u);
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==0); /* EE cleared on entry */
  ppcvm_irq_latch_clear(&latch,1u);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK); /* rfi */
  assert(cpu.pc==0x1000u && (cpu.msr&0x8000u));
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==0);
  ppcvm_irq_latch_set(&latch,2u);
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==0); /* source 2 masked */
  ppcvm_irq_latch_enable(&latch,2u);
  assert(ppcvm_irq_bridge_poll(&cpu,&latch)==1);
  assert(cpu.srr0==0x1000u);
  assert(ppcvm_irq_bridge_poll(0,&latch)==-1);
  assert(ppcvm_irq_bridge_poll(&cpu,0)==-1);
  return 0;
}
