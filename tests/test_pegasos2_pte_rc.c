#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x123);
  uint32_t pteg=0,pte1=0;
  uint32_t ea=UINT32_C(0x4000);
  assert(ppcvm_mmu_pteg_address(&m.segments,ea,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa002))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x90640000))==PPCVM_MEM_OK);
  m.cpu.msr=UINT32_C(0x10);
  m.cpu.gpr[4]=ea;
  m.cpu.gpr[3]=UINT32_C(0x12345678);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa182));
  /* A denied write must not set C. */
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  m.cpu.pc=0;
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
