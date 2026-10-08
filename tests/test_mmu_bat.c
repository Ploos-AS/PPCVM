#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bat_state b={0};
  uint32_t pa=0;
  b.ibatu[0]=UINT32_C(0x80000002); /* 128 KiB, supervisor valid */
  b.ibatl[0]=UINT32_C(0x10000000);
  assert(ppcvm_mmu_translate_bat(&b,0x20,0x80001234,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x10001234));
  assert(ppcvm_mmu_translate_bat(&b,0x4020,0x80001234,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_translate_bat(&b,0x20,0x80020000,PPCVM_ACCESS_INSTRUCTION,&pa)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_translate_bat(&b,0x10,0x80001234,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_UNSUPPORTED);
  b.dbatu[0]=UINT32_C(0x90000007); /* 256 KiB, user and supervisor valid */
  b.dbatl[0]=UINT32_C(0x20000000);
  assert(ppcvm_mmu_translate_bat(&b,0x4010,0x90034567,PPCVM_ACCESS_DATA_WRITE,&pa)==PPCVM_MMU_OK);
  assert(pa==UINT32_C(0x20034567));
  assert(ppcvm_mmu_translate_bat(&b,0,0x1234,PPCVM_ACCESS_DATA_READ,&pa)==PPCVM_MMU_OK && pa==0x1234);
  return 0;
}
