#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define BCCTR(bo,bi,lk) ((19u<<26)|((uint32_t)(bo)<<21)|((uint32_t)(bi)<<16)|(528u<<1)|(lk))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  c.pc=0x100; c.ctr=0x203; c.cr=0x80000000u;
  assert(ppcvm_cpu_step(&c,BCCTR(20,0,0))==PPCVM_OK);
  assert(c.pc==0x200 && c.ctr==0x203);
  c.pc=0x100; c.ctr=0x300;
  assert(ppcvm_cpu_step(&c,BCCTR(12,0,1))==PPCVM_OK);
  assert(c.pc==0x300 && c.lr==0x104 && c.ctr==0x300);
  c.pc=0x100; c.cr=0;
  assert(ppcvm_cpu_step(&c,BCCTR(12,0,0))==PPCVM_OK && c.pc==0x104);
  c.pc=0x100;
  assert(ppcvm_cpu_step(&c,BCCTR(16,0,0))==PPCVM_UNSUPPORTED);
  assert(c.pc==0x100 && c.ctr==0x300);
  return 0;
}
