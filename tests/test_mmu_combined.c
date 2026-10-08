#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_memory ram;
  ppcvm_bat_state bat={0};
  ppcvm_segment_state segments={0};
  assert(ppcvm_memory_init(&ram,131072)==PPCVM_MEM_OK);
  segments.sr[0]=UINT32_C(0x123);
  uint32_t ea=UINT32_C(0x4567),pa=0,pteg=0;
  assert(ppcvm_mmu_translate_combined(&bat,&segments,&ram,0,ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(pa==ea);
  assert(ppcvm_mmu_pteg_address(&segments,ea,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&ram,pteg+4,UINT32_C(0xa002))==PPCVM_MEM_OK);
  assert(ppcvm_mmu_translate_combined(&bat,&segments,&ram,UINT32_C(0x10),ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0xa567));
  bat.dbatu[0]=UINT32_C(0x00000002);
  bat.dbatl[0]=UINT32_C(0x00000000);
  pa=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_translate_combined(&bat,&segments,&ram,UINT32_C(0x10),ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_PROTECTION);
  assert(pa==UINT32_C(0xdeadbeef));
  bat.dbatl[0]=UINT32_C(0x00000002);
  assert(ppcvm_mmu_translate_combined(&bat,&segments,&ram,UINT32_C(0x10),ea,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(pa==ea);
  ppcvm_memory_free(&ram);
  return 0;
}
