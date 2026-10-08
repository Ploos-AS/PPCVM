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

ppcvm_mmu_result ppcvm_mmu_translate_bat(const ppcvm_bat_state *state,
                                         uint32_t msr, uint32_t ea,
                                         ppcvm_access access, uint32_t *pa) {
  if (!pa || !state || (access!=PPCVM_ACCESS_INSTRUCTION &&
                         access!=PPCVM_ACCESS_DATA_READ &&
                         access!=PPCVM_ACCESS_DATA_WRITE))
    return PPCVM_MMU_UNSUPPORTED;
  if (!(msr & (access==PPCVM_ACCESS_INSTRUCTION ? MSR_IR : MSR_DR))) {
    *pa=ea;
    return PPCVM_MMU_OK;
  }
  const uint32_t *upper=access==PPCVM_ACCESS_INSTRUCTION ? state->ibatu : state->dbatu;
  const uint32_t *lower=access==PPCVM_ACCESS_INSTRUCTION ? state->ibatl : state->dbatl;
  uint32_t valid=(msr & UINT32_C(0x4000)) ? 1u : 2u; /* PR: VP vs VS */
  for (unsigned i=0;i<4;i++) {
    uint32_t bl=(upper[i]>>2)&0x7ffu;
    /* Valid BAT block lengths have contiguous low-order ones. */
    if ((bl & (bl+1u))!=0u || !(upper[i]&valid)) continue;
    uint32_t mask=(bl<<17)|UINT32_C(0x1ffff);
    if ((ea & ~mask)!=(upper[i]&UINT32_C(0xfffe0000)&~mask)) continue;
    /* BAT PP=00 denies access; other PP combinations need full privilege rules. */
    if (access!=PPCVM_ACCESS_INSTRUCTION && (lower[i]&3u)==0u)
      return PPCVM_MMU_PROTECTION;
    *pa=(lower[i]&UINT32_C(0xfffe0000)&~mask)|(ea&mask);
    return PPCVM_MMU_OK;
  }
  return PPCVM_MMU_UNSUPPORTED; /* No BAT match; page tables unavailable */
}
