#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_memory ram;
  ppcvm_segment_state s={0};
  assert(ppcvm_memory_init(&ram,131072)==PPCVM_MEM_OK);
  s.sr[0]=UINT32_C(0x123);
  uint32_t ea=UINT32_C(0x4567),pteg=0,pa=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_pteg_address(&s,ea,0,&pteg)==PPCVM_MMU_OK);
  uint32_t pte0=UINT32_C(0x80000000)|(UINT32_C(0x123)<<7);
  assert(ppcvm_memory_write32be(&ram,pteg,pte0)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+4,UINT32_C(0xa000))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte_access(&s,&ram,ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_PROTECTION);
  assert(pa==UINT32_C(0xdeadbeef));
  assert(ppcvm_memory_write32be(&ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte_access(&s,&ram,ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0xa567));
  pa=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_lookup_pte_access(&s,&ram,ea,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_PROTECTION);
  assert(pa==UINT32_C(0xdeadbeef));
  assert(ppcvm_memory_write32be(&ram,pteg+4,UINT32_C(0xa002))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte_access(&s,&ram,ea,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0xa567));
  assert(ppcvm_mmu_lookup_pte_access(&s,&ram,ea,(ppcvm_access)-1,&pa)==PPCVM_MMU_UNSUPPORTED);
  ppcvm_memory_free(&ram);
  return 0;
}
