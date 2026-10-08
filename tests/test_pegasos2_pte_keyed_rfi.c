#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x20000123);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x90640000))==PPCVM_MEM_OK); /* stw r3,0(r4) */
  assert(ppcvm_memory_write32be(&m.ram,PPCVM_VECTOR_DSI,UINT32_C(0x4c000064))==PPCVM_MEM_OK); /* rfi */
  uint32_t pteg=0;
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  m.cpu.msr=UINT32_C(0x4010);
  m.cpu.gpr[4]=UINT32_C(0x4000);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.srr0==0);
  assert(m.cpu.srr1==UINT32_C(0x4010));
  assert((m.cpu.msr&UINT32_C(0x4010))==0);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==0);
  assert(m.cpu.msr==UINT32_C(0x4010));
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI); /* denied store remains denied after rfi */
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
