#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  cpu.pc=UINT32_C(0x1000);
  cpu.msr=UINT32_C(0x0000c030);
  cpu.gpr[3]=5;
  /* twi TO=4, RA=3, SI=5: equality trap */
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x0c830005))==PPCVM_OK);
  assert(cpu.pc==PPCVM_VECTOR_PROGRAM);
  assert(cpu.srr0==UINT32_C(0x1000));
  assert(cpu.srr1==UINT32_C(0x0002c030));
  assert((cpu.msr&UINT32_C(0x0000c030))==0);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(cpu.pc==UINT32_C(0x1000));
  assert(cpu.msr==UINT32_C(0x0002c030)); /* cause bit is retained by current rfi model */
  ppcvm_cpu_reset(&cpu);
  cpu.pc=UINT32_C(0x2000);
  cpu.msr=UINT32_C(0x0000c030);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x44000002))==PPCVM_OK); /* sc */
  assert(cpu.pc==PPCVM_VECTOR_SYSCALL);
  assert(cpu.srr0==UINT32_C(0x2004));
  assert(cpu.srr1==UINT32_C(0x0000c030));
  assert((cpu.msr&UINT32_C(0x0000c030))==0);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(cpu.pc==UINT32_C(0x2004));
  assert(cpu.msr==UINT32_C(0x0000c030));
  return 0;
}
