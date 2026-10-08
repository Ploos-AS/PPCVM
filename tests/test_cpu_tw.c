#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define TW(to,ra,rb) ((31u<<26)|((uint32_t)(to)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(rb)<<11)|(4u<<1))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x100; c.gpr[3]=5; c.gpr[4]=6;
  assert(ppcvm_cpu_step(&c,TW(4,3,4))==PPCVM_OK && c.pc==0x104);
  c.msr=UINT32_C(0x0000c030);
  assert(ppcvm_cpu_step(&c,TW(16,3,4))==PPCVM_OK);
  assert(c.pc==PPCVM_VECTOR_PROGRAM && c.srr0==0x104);
  assert((c.srr1&UINT32_C(0x00020000))!=0 && c.msr==0);
  c.srr0+=4;
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_OK && c.pc==0x108);
  ppcvm_cpu_reset(&c);
  c.gpr[3]=UINT32_C(0xffffffff); c.gpr[4]=1;
  assert(ppcvm_cpu_step(&c,TW(2,3,4))==PPCVM_OK && c.pc==4);
  assert(ppcvm_cpu_step(&c,TW(16,3,4)|1u)==PPCVM_UNSUPPORTED && c.pc==4);
  return 0;
}
