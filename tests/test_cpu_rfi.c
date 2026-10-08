#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x700;
  c.srr0=0x1237;
  c.srr1=0x2030;
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(c.pc==0x1234 && c.msr==0x2030);
  c.pc=0x700; c.msr=UINT32_C(0x4000); c.srr0=0x2000; c.srr1=0;
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_UNSUPPORTED);
  assert(c.pc==0x700 && c.msr==UINT32_C(0x4000));
  c.msr=0;
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000065))==PPCVM_UNSUPPORTED);
  assert(c.pc==0x700);
  return 0;
}
