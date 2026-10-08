#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_memory ram;
  ppcvm_segment_state s={0};
  assert(ppcvm_memory_init(&ram,131072)==PPCVM_MEM_OK);
  s.sr[0]=UINT32_C(0x00000123);
  uint32_t ea=UINT32_C(0x00004567), pteg=0,pa=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_pteg_address(&s,ea,0,&pteg)==PPCVM_MMU_OK);
  uint32_t pte0=UINT32_C(0x80000000)|(UINT32_C(0x123)<<7)|((ea>>22)&63u);
  assert(ppcvm_memory_write32be(&ram,pteg+16,pte0)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+20,UINT32_C(0x0000a002))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte(&s,&ram,ea,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x0000a567));
  assert(ppcvm_memory_write32be(&ram,pteg+16,0)==PPCVM_MEM_OK);
  pa=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_lookup_pte(&s,&ram,ea,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(pa==UINT32_C(0xdeadbeef));
  assert(ppcvm_mmu_pteg_address(&s,ea,1,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+56,pte0|UINT32_C(0x40))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+60,UINT32_C(0x0000b002))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_lookup_pte(&s,&ram,ea,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x0000b567));
  assert(ppcvm_mmu_lookup_pte(&s,&ram,ea,0)==PPCVM_MMU_UNSUPPORTED);
  ppcvm_memory_free(&ram);
  return 0;
}
