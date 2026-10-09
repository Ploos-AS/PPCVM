#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  assert(ppcvm_cpu_accept_external_irq(0)==-1);
  cpu.pc=0x1234u;
  cpu.msr=0;
  assert(ppcvm_cpu_accept_external_irq(&cpu)==0);
  assert(cpu.pc==0x1234u && cpu.srr0==0);
  cpu.msr=0x8000u;
  assert(ppcvm_cpu_accept_external_irq(&cpu)==1);
  assert(cpu.pc==PPCVM_VECTOR_EXTERNAL);
  assert(cpu.srr0==0x1234u && cpu.srr1==0x8000u);
  assert((cpu.msr&0x8000u)==0);
  assert(ppcvm_cpu_accept_external_irq(&cpu)==0);
  ppcvm_cpu_reset(&cpu);
  cpu.pc=0x2000u;
  cpu.msr=0x8040u;
  assert(ppcvm_cpu_accept_external_irq(&cpu)==1);
  assert(cpu.pc==0xfff00500u);
  assert(cpu.srr0==0x2000u && cpu.srr1==0x8040u);
  assert((cpu.msr&0x8000u)==0);
  return 0;
}
