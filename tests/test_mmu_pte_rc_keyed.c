#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_memory ram;
  ppcvm_segment_state s={0};
  assert(ppcvm_memory_init(&ram,131072)==PPCVM_MEM_OK);
  s.sr[0]=UINT32_C(0x20000123); /* Ks=0 Kp=1 */
  uint32_t ea=UINT32_C(0x4567),pteg=0,pa=0,pte1=0;
  assert(ppcvm_mmu_pteg_address(&s,ea,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte_rc_keyed(&s,&ram,UINT32_C(0x4000),ea,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_PROTECTION);
  assert(ppcvm_memory_read32be(&ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001));
  assert(ppcvm_mmu_lookup_pte_rc_keyed(&s,&ram,0,ea,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0xa567));
  assert(ppcvm_memory_read32be(&ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa181));
  ppcvm_memory_free(&ram);
  return 0;
}
