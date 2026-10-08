#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define X(rt,ra,rb,xo,rc) ((31u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(rb)<<11)|((uint32_t)(xo)<<1)|(rc))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.gpr[3]=10; c.gpr[4]=4;
  assert(ppcvm_cpu_step(&c,X(5,3,4,266,0))==PPCVM_OK && c.gpr[5]==14);
  assert(ppcvm_cpu_step(&c,X(6,3,4,40,0))==PPCVM_OK && c.gpr[6]==UINT32_C(0xfffffffa));
  c.gpr[7]=UINT32_C(0xf0f0);
  c.gpr[8]=UINT32_C(0x0ff0);
  assert(ppcvm_cpu_step(&c,X(7,9,8,444,0))==PPCVM_OK && c.gpr[9]==UINT32_C(0xfff0));
  assert(ppcvm_cpu_step(&c,X(7,10,8,316,0))==PPCVM_OK && c.gpr[10]==UINT32_C(0xff00));
  assert(ppcvm_cpu_step(&c,X(7,11,8,28,1))==PPCVM_OK && c.gpr[11]==UINT32_C(0x00f0));
  assert((c.cr>>28)==4u);
  c.gpr[3]=UINT32_MAX; c.gpr[4]=1; c.xer=UINT32_C(0x80000000);
  assert(ppcvm_cpu_step(&c,X(5,3,4,266,1))==PPCVM_OK && c.gpr[5]==0 && (c.cr>>28)==3u);
  uint32_t pc=c.pc;
  assert(ppcvm_cpu_step(&c,X(5,3,4,266,0)|0x400u)==PPCVM_UNSUPPORTED && c.pc==pc);
  return 0;
}
