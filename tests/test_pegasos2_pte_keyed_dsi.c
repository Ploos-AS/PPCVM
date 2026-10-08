#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t pteg=0,pte1=0,value=0;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x20000123);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x90640000))==PPCVM_MEM_OK); /* stw */
  m.cpu.msr=UINT32_C(0x4010);
  m.cpu.gpr[3]=UINT32_C(0xdeadbeef);
  m.cpu.gpr[4]=UINT32_C(0x4000);
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.srr0==0);
  assert(m.cpu.dar==UINT32_C(0x4000));
  assert(m.cpu.dsisr==UINT32_C(0x0a000000));
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001));
  assert(ppcvm_memory_read32be(&m.ram,UINT32_C(0xa000),&value)==PPCVM_MEM_OK);
  assert(value==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
