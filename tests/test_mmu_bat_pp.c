#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bat_state b={0};
  uint32_t pa=UINT32_C(0xdeadbeef);
  b.dbatu[0]=UINT32_C(0x90000003);
  b.dbatl[0]=UINT32_C(0x20000000); /* PP=00 */
  assert(ppcvm_mmu_translate_bat(&b,0x10,0x90000100,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(pa==UINT32_C(0xdeadbeef));
  assert(ppcvm_mmu_translate_bat(&b,0x4010,0x90000100,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_UNSUPPORTED);
  b.dbatl[0]=UINT32_C(0x20000002);
  assert(ppcvm_mmu_translate_bat(&b,0x10,0x90000100,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x20000100));
  return 0;
}
