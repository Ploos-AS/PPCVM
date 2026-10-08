#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x100;
  c.msr=UINT32_C(0x0000c030);
  assert(ppcvm_cpu_step(&c,UINT32_C(0x44000002))==PPCVM_OK);
  assert(c.pc==0xc00 && c.srr0==0x104 && c.srr1==UINT32_C(0x0000c030));
  assert(c.msr==0);
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(c.pc==0x104 && c.msr==UINT32_C(0x0000c030));
  uint32_t oldpc=c.pc, oldmsr=c.msr;
  assert(ppcvm_cpu_step(&c,UINT32_C(0x44000003))==PPCVM_UNSUPPORTED);
  assert(c.pc==oldpc && c.msr==oldmsr);
  return 0;
}
