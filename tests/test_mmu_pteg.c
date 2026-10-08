#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_segment_state s={0};
  uint32_t address=UINT32_C(0xdeadbeef);
  s.sr[8]=UINT32_C(0x00012345);
  s.sdr1=UINT32_C(0x00100000);
  uint32_t hash=(UINT32_C(0x12345)&UINT32_C(0x7ffff))^UINT32_C(0x12);
  uint32_t primary=UINT32_C(0x00100000)|((hash&UINT32_C(0x3ff))<<6);
  uint32_t secondary=UINT32_C(0x00100000)|(((~hash)&UINT32_C(0x3ff))<<6);
  assert(ppcvm_mmu_pteg_address(&s,UINT32_C(0x80012000),0,&address)==PPCVM_MMU_OK);
  assert(address==primary);
  assert(ppcvm_mmu_pteg_address(&s,UINT32_C(0x80012000),1,&address)==PPCVM_MMU_OK);
  assert(address==secondary);
  s.sdr1=UINT32_C(0x00200001);
  assert(ppcvm_mmu_pteg_address(&s,UINT32_C(0x80012000),0,&address)==PPCVM_MMU_OK);
  assert(address==(UINT32_C(0x00200000)|((hash&UINT32_C(0x7ff))<<6)));
  s.sr[8]=UINT32_C(0x80000000);
  address=UINT32_C(0xdeadbeef);
  assert(ppcvm_mmu_pteg_address(&s,UINT32_C(0x80012000),0,&address)==PPCVM_MMU_UNSUPPORTED);
  assert(address==UINT32_C(0xdeadbeef));
  assert(ppcvm_mmu_pteg_address(&s,0,2,&address)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_pteg_address(0,0,0,&address)==PPCVM_MMU_UNSUPPORTED);
  return 0;
}
