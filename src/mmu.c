#include "ppcvm/mmu.h"
#include <stddef.h>
#define MSR_IR UINT32_C(0x20)
#define MSR_DR UINT32_C(0x10)
ppcvm_mmu_result ppcvm_mmu_translate(uint32_t msr, uint32_t ea,
                                     ppcvm_access access, uint32_t *pa) {
  if (!pa) return PPCVM_MMU_UNSUPPORTED;
  if (access!=PPCVM_ACCESS_INSTRUCTION && access!=PPCVM_ACCESS_DATA_READ &&
      access!=PPCVM_ACCESS_DATA_WRITE) return PPCVM_MMU_UNSUPPORTED;
  if (msr & (access==PPCVM_ACCESS_INSTRUCTION ? MSR_IR : MSR_DR))
    return PPCVM_MMU_UNSUPPORTED; /* BAT/page tables not implemented */
  *pa=ea;
  return PPCVM_MMU_OK;
}
