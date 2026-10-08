#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define CMP(op,bf,ra,imm) (((uint32_t)(op)<<26)|((uint32_t)(bf)<<23)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
#define BC(bo,bi,bd) ((16u<<26)|((uint32_t)(bo)<<21)|((uint32_t)(bi)<<16)|((uint32_t)(bd)&0xfffcu))
#define BCLR(bo,bi,lk) ((19u<<26)|((uint32_t)(bo)<<21)|((uint32_t)(bi)<<16)|(16u<<1)|(lk))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.gpr[3]=UINT32_MAX;
  assert(ppcvm_cpu_step(&c,CMP(11,0,3,0))==PPCVM_OK);
  assert((c.cr>>28)==8u);
  assert(ppcvm_cpu_step(&c,CMP(10,1,3,0))==PPCVM_OK);
  assert(((c.cr>>24)&15u)==4u && (c.cr>>28)==8u);
  c.pc=0x100;
  assert(ppcvm_cpu_step(&c,BC(12,0,8))==PPCVM_OK && c.pc==0x108);
  assert(ppcvm_cpu_step(&c,BC(4,0,8))==PPCVM_OK && c.pc==0x10c);
  c.ctr=2; c.pc=0x200;
  assert(ppcvm_cpu_step(&c,BC(16,0,0xfffcu))==PPCVM_OK && c.pc==0x1fc && c.ctr==1);
  assert(ppcvm_cpu_step(&c,BC(16,0,0xfffcu))==PPCVM_OK && c.pc==0x200 && c.ctr==0);
  c.lr=0x304; c.pc=0x220;
  assert(ppcvm_cpu_step(&c,BCLR(20,0,0))==PPCVM_OK && c.pc==0x304);
  c.lr=0x408; c.pc=0x300;
  assert(ppcvm_cpu_step(&c,BCLR(20,0,1))==PPCVM_OK && c.pc==0x408 && c.lr==0x304);
  return 0;
}
