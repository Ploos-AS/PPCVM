#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define SPR(op,rt,spr) ((31u<<26)|((uint32_t)(rt)<<21)|(((uint32_t)(spr)&31u)<<16)|(((uint32_t)(spr)>>5)<<11)|((uint32_t)(op)<<1))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.gpr[3]=0x12345678u;
  c.gpr[4]=0x10203040u;
  assert(ppcvm_cpu_step(&c,SPR(467,3,8))==PPCVM_OK && c.lr==0x12345678u);
  assert(ppcvm_cpu_step(&c,SPR(467,4,9))==PPCVM_OK && c.ctr==0x10203040u);
  assert(ppcvm_cpu_step(&c,SPR(339,5,8))==PPCVM_OK && c.gpr[5]==c.lr);
  assert(ppcvm_cpu_step(&c,SPR(339,6,9))==PPCVM_OK && c.gpr[6]==c.ctr);
  uint32_t pc=c.pc;
  assert(ppcvm_cpu_step(&c,SPR(339,5,1))==PPCVM_UNSUPPORTED && c.pc==pc);
  assert(ppcvm_cpu_step(&c,SPR(467,5,1))==PPCVM_UNSUPPORTED && c.pc==pc);
  return 0;
}
