#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x100;
  c.msr=UINT32_C(0x0000c030);
  c.gpr[3]=123;
  ppcvm_cpu_enter_exception(&c,PPCVM_VECTOR_PROGRAM,c.pc);
  assert(c.pc==PPCVM_VECTOR_PROGRAM);
  assert(c.srr0==0x100 && c.srr1==UINT32_C(0x0000c030));
  assert(c.msr==0 && c.gpr[3]==123);
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(c.pc==0x100 && c.msr==UINT32_C(0x0000c030));
  ppcvm_cpu_enter_exception(&c,PPCVM_VECTOR_SYSCALL,0x104);
  assert(c.pc==PPCVM_VECTOR_SYSCALL && c.srr0==0x104);
  return 0;
}
