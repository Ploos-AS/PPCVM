#include "ppcvm/mmu.h"
#include <assert.h>
int main(void) {
  const int read_ok[2][4]={{1,1,1,1},{0,1,1,1}};
  const int write_ok[2][4]={{1,1,1,0},{0,0,1,0}};
  for (unsigned key=0;key<2;key++)
    for (unsigned pp=0;pp<4;pp++) {
      assert((ppcvm_mmu_check_pte_permission(key,pp,PPCVM_ACCESS_DATA_READ)==PPCVM_MMU_OK)==read_ok[key][pp]);
      assert((ppcvm_mmu_check_pte_permission(key,pp,PPCVM_ACCESS_INSTRUCTION)==PPCVM_MMU_OK)==read_ok[key][pp]);
      assert((ppcvm_mmu_check_pte_permission(key,pp,PPCVM_ACCESS_DATA_WRITE)==PPCVM_MMU_OK)==write_ok[key][pp]);
    }
  assert(ppcvm_mmu_check_pte_permission(2,0,PPCVM_ACCESS_DATA_READ)==PPCVM_MMU_UNSUPPORTED);
  assert(ppcvm_mmu_check_pte_permission(0,4,PPCVM_ACCESS_DATA_READ)==PPCVM_MMU_UNSUPPORTED);
  return 0;
}
