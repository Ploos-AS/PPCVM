#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define SR_INSN(xo,rt,sr) ((31u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(sr)<<16)|((uint32_t)(xo)<<1))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.cpu.gpr[3]=UINT32_C(0x00012345);
  assert(ppcvm_memory_write32be(&m.ram,0,SR_INSN(210,3,9))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,4,SR_INSN(595,4,9))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.segments.sr[9]==UINT32_C(0x00012345));
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.gpr[4]==UINT32_C(0x00012345));
  uint32_t vsid=0;
  assert(ppcvm_mmu_segment_vsid(&m.segments,UINT32_C(0x90001234),&vsid)==PPCVM_MMU_OK);
  assert(vsid==UINT32_C(0x00012345));
  m.cpu.pc=0;
  m.cpu.msr=UINT32_C(0x4000);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_PROGRAM);
  assert(m.cpu.srr0==0 && (m.cpu.srr1&UINT32_C(0x00040000))!=0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
