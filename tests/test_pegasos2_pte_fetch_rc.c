#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x123);
  uint32_t pteg=0,pte1=0;
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa002))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,UINT32_C(0xa000),UINT32_C(0x3860002a))==PPCVM_MEM_OK);
  m.cpu.pc=UINT32_C(0x4000);
  m.cpu.msr=UINT32_C(0x20);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa102)); /* fetch sets R, not C */
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
