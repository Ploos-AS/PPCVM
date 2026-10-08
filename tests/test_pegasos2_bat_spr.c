#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define SPR_INSN(xo,rt,spr) ((31u<<26)|((uint32_t)(rt)<<21)|(((uint32_t)(spr)&31u)<<16)|((((uint32_t)(spr)>>5)&31u)<<11)|((uint32_t)(xo)<<1))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.cpu.gpr[3]=UINT32_C(0x80000002);
  assert(ppcvm_memory_write32be(&m.ram,0,SPR_INSN(467,3,528))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,4,SPR_INSN(339,4,528))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,8,SPR_INSN(467,3,536))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.bat.ibatu[0]==UINT32_C(0x80000002));
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.gpr[4]==UINT32_C(0x80000002));
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.bat.dbatu[0]==UINT32_C(0x80000002));
  m.cpu.pc=0;
  m.cpu.msr=UINT32_C(0x4000);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_UNSUPPORTED);
  assert(m.cpu.pc==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
