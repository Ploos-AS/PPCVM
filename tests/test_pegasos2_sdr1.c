#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define SPR_INSN(xo,rt,spr) ((31u<<26)|((uint32_t)(rt)<<21)|(((uint32_t)(spr)&31u)<<16)|((((uint32_t)(spr)>>5)&31u)<<11)|((uint32_t)(xo)<<1))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.cpu.gpr[3]=UINT32_C(0x00100000);
  assert(ppcvm_memory_write32be(&m.ram,0,SPR_INSN(467,3,25))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,4,SPR_INSN(339,4,25))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.segments.sdr1==UINT32_C(0x00100000));
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.gpr[4]==UINT32_C(0x00100000));
  m.cpu.pc=0;
  m.cpu.msr=UINT32_C(0x4000);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_PROGRAM);
  assert(m.cpu.srr0==0 && (m.cpu.srr1&UINT32_C(0x00040000))!=0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
