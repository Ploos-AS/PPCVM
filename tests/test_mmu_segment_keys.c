#include "ppcvm/mmu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_segment_state s={0};
  uint32_t key=99;
  s.sr[0]=UINT32_C(0x40000123); /* Ks=1, Kp=0 */
  assert(ppcvm_mmu_segment_key(&s,0,UINT32_C(0x4567),&key)==PPCVM_MMU_OK);
  assert(key==1);
  assert(ppcvm_mmu_segment_key(&s,UINT32_C(0x4000),UINT32_C(0x4567),&key)==PPCVM_MMU_OK);
  assert(key==0);
  s.sr[0]=UINT32_C(0x20000123); /* Ks=0, Kp=1 */
  assert(ppcvm_mmu_segment_key(&s,0,UINT32_C(0x4567),&key)==PPCVM_MMU_OK);
  assert(key==0);
  assert(ppcvm_mmu_segment_key(&s,UINT32_C(0x4000),UINT32_C(0x4567),&key)==PPCVM_MMU_OK);
  assert(key==1);
  s.sr[0]=UINT32_C(0x80000123); /* T=1: unsupported direct-store */
  assert(ppcvm_mmu_segment_key(&s,0,UINT32_C(0x4567),&key)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_segment_key(0,0,0,&key)==PPCVM_MMU_UNSUPPORTED);
  return 0;
}
