#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_segment_state s={0};
  uint32_t vsid=UINT32_C(0xdeadbeef);
  s.sr[0]=UINT32_C(0x00012345);
  s.sr[9]=UINT32_C(0x400abcde);
  s.sr[15]=UINT32_C(0x80000001);
  assert(ppcvm_mmu_segment_vsid(&s,UINT32_C(0x00001000),&vsid)==PPCVM_MMU_OK);
  assert(vsid==UINT32_C(0x00012345));
  assert(ppcvm_mmu_segment_vsid(&s,UINT32_C(0x9fffffff),&vsid)==PPCVM_MMU_OK);
  assert(vsid==UINT32_C(0x000abcde));
  assert(ppcvm_mmu_segment_vsid(&s,UINT32_C(0xf0001000),&vsid)==PPCVM_MMU_UNSUPPORTED);
  assert(vsid==UINT32_C(0x000abcde));
  assert(ppcvm_mmu_segment_vsid(0,0,&vsid)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_segment_vsid(&s,0,0)==PPCVM_MMU_UNSUPPORTED);
  return 0;
}
