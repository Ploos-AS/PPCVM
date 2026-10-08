#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  cpu.pc=UINT32_C(0x1000);
  cpu.msr=UINT32_C(0x00008030);
  cpu.srr1=UINT32_C(0xffffffff);
  cpu.gpr[3]=7;
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x0c830007))==PPCVM_OK); /* twi equal */
  assert(cpu.pc==PPCVM_VECTOR_PROGRAM);
  assert(cpu.srr0==UINT32_C(0x1000));
  assert(cpu.srr1==UINT32_C(0x00028030));
  /* New exception must replace stale SRR1 contents, not OR into them. */
  cpu.pc=UINT32_C(0x2000);
  cpu.msr=UINT32_C(0x00008030);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x44000002))==PPCVM_OK);
  assert(cpu.pc==PPCVM_VECTOR_SYSCALL);
  assert(cpu.srr0==UINT32_C(0x2004));
  assert(cpu.srr1==UINT32_C(0x00008030));
  assert((cpu.srr1&UINT32_C(0x00020000))==0);
  /* A subsequent trap must set its own cause again. */
  cpu.pc=UINT32_C(0x3000);
  cpu.msr=UINT32_C(0x00008030);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x0c830007))==PPCVM_OK);
  assert(cpu.srr1==UINT32_C(0x00028030));
  return 0;
}
