#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define RL(rs,ra,sh,mb,me,rc) ((21u<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(sh)<<11)|((uint32_t)(mb)<<6)|((uint32_t)(me)<<1)|(rc))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.gpr[3]=UINT32_C(0x12345678);
  assert(ppcvm_cpu_step(&c,RL(3,4,0,0,31,0))==PPCVM_OK && c.gpr[4]==UINT32_C(0x12345678));
  assert(ppcvm_cpu_step(&c,RL(3,4,8,0,31,0))==PPCVM_OK && c.gpr[4]==UINT32_C(0x34567812));
  assert(ppcvm_cpu_step(&c,RL(3,4,0,8,15,0))==PPCVM_OK && c.gpr[4]==UINT32_C(0x00340000));
  c.gpr[3]=UINT32_C(0xffffffff);
  assert(ppcvm_cpu_step(&c,RL(3,4,0,28,3,1))==PPCVM_OK && c.gpr[4]==UINT32_C(0xf000000f));
  assert((c.cr>>28)==8u);
  c.gpr[3]=0;
  c.xer=UINT32_C(0x80000000);
  assert(ppcvm_cpu_step(&c,RL(3,4,0,0,31,1))==PPCVM_OK && c.gpr[4]==0);
  assert((c.cr>>28)==3u);
  return 0;
}
