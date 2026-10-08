#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x20000123); /* Ks=0, Kp=1 */
  uint32_t pteg=0,pte1=0;
  assert(ppcvm_mmu_pteg_address(&m.segments,0,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa000))==PPCVM_MEM_OK); /* PP=0: key 1 denied */
  assert(ppcvm_memory_write32be(&m.ram,UINT32_C(0xa000),UINT32_C(0x60000000))==PPCVM_MEM_OK);
  m.cpu.msr=UINT32_C(0x4020); /* problem mode + IR */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_ISI);
  assert(m.cpu.srr0==0);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa000)); /* rejected fetch must not set R */
  ppcvm_pegasos2_destroy(&m);
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x20000123);
  assert(ppcvm_mmu_pteg_address(&m.segments,0,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,pteg+4,UINT32_C(0xa000))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,UINT32_C(0xa000),UINT32_C(0x60000000))==PPCVM_MEM_OK);
  m.cpu.msr=UINT32_C(0x20); /* supervisor key 0 permits PP 0 */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==4);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa100));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
