#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bat_state bat={0};
  uint32_t pa=UINT32_C(0xfeedbeef);
  bat.dbatu[0]=UINT32_C(0x90000003);
  bat.dbatl[0]=UINT32_C(0x20000002);
  assert(ppcvm_mmu_translate_bat(&bat,0x10,0x90000100,(ppcvm_access)-1,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(pa==UINT32_C(0xfeedbeef));
  assert(ppcvm_mmu_translate_bat(&bat,0x10,0x90000100,(ppcvm_access)3,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(pa==UINT32_C(0xfeedbeef));
  /* Invalid, non-contiguous BAT block-length encoding must never match. */
  bat.dbatu[0]=UINT32_C(0x9000000b);
  assert(ppcvm_mmu_translate_bat(&bat,0x10,0x90000100,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(pa==UINT32_C(0xfeedbeef));
  return 0;
}
