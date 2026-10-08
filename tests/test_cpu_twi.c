#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define TWI(to,ra,imm) ((3u<<26)|((uint32_t)(to)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x100; c.gpr[3]=5;
  assert(ppcvm_cpu_step(&c,TWI(4,3,6))==PPCVM_OK && c.pc==0x104);
  c.msr=UINT32_C(0x0000c030);
  assert(ppcvm_cpu_step(&c,TWI(16,3,6))==PPCVM_OK);
  assert(c.pc==PPCVM_VECTOR_PROGRAM && c.srr0==0x104);
  assert((c.srr1&UINT32_C(0x00020000))!=0);
  assert((c.srr1&UINT32_C(0x0000c030))==UINT32_C(0x0000c030));
  assert(c.msr==0);
  c.srr0+=4; /* handler skips trapping instruction */
  assert(ppcvm_cpu_step(&c,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(c.pc==0x108 && (c.msr&UINT32_C(0x0000c030))==UINT32_C(0x0000c030));
  ppcvm_cpu_reset(&c); c.gpr[3]=UINT32_C(0xffffffff);
  assert(ppcvm_cpu_step(&c,TWI(2,3,1))==PPCVM_OK && c.pc==4);
  assert(ppcvm_cpu_step(&c,TWI(16,3,1))==PPCVM_OK && c.pc==PPCVM_VECTOR_PROGRAM);
  return 0;
}
