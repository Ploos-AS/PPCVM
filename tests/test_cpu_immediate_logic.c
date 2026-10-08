#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define I(op,rs,ra,imm) (((uint32_t)(op)<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.gpr[3]=UINT32_C(0x12345678);
  assert(ppcvm_cpu_step(&c,I(27,3,4,0xffff))==PPCVM_OK);
  assert(c.gpr[4]==UINT32_C(0xedcb5678));
  assert(ppcvm_cpu_step(&c,I(29,3,5,0x1234))==PPCVM_OK);
  assert(c.gpr[5]==UINT32_C(0x12340000) && (c.cr>>28)==4u);
  c.xer=UINT32_C(0x80000000);
  assert(ppcvm_cpu_step(&c,I(29,3,5,0))==PPCVM_OK);
  assert(c.gpr[5]==0 && (c.cr>>28)==3u);
  c.gpr[3]=UINT32_C(0x80000000);
  assert(ppcvm_cpu_step(&c,I(29,3,5,0x8000))==PPCVM_OK);
  assert(c.gpr[5]==UINT32_C(0x80000000) && (c.cr>>28)==9u);
  return 0;
}
