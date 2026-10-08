#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  uint32_t pa=0;
  assert(ppcvm_mmu_translate(0,0x12345678,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x12345678));
  assert(ppcvm_mmu_translate(0x20,0x1000,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_translate(0x20,0x1000,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(ppcvm_mmu_translate(0x10,0x1000,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_translate(0x10,0x1000,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_OK);
  assert(ppcvm_mmu_translate(0,0,(ppcvm_access)99,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_translate(0,0,PPCVM_ACCESS_DATA_READ,0)==PPCVM_MMU_UNSUPPORTED);
  return 0;
}
