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
    /* Initial BAT protection: PP=00 denies data; PP=01 denies writes.
       Full key/privilege semantics remain to be modeled. */
    uint32_t pp=lower[i]&3u;
    if (access!=PPCVM_ACCESS_INSTRUCTION &&
        (pp==0u || (pp==1u && access==PPCVM_ACCESS_DATA_WRITE)))
      return PPCVM_MMU_PROTECTION;
    *pa=(lower[i]&UINT32_C(0xfffe0000)&~mask)|(ea&mask);
    return PPCVM_MMU_OK;
  }
  return PPCVM_MMU_UNSUPPORTED; /* No BAT match; page tables unavailable */
}

/* Segment lookup only: does not imply successful page translation. */
ppcvm_mmu_result ppcvm_mmu_segment_vsid(const ppcvm_segment_state *state,
                                         uint32_t ea, uint32_t *vsid) {
  if (!state || !vsid) return PPCVM_MMU_UNSUPPORTED;
  uint32_t sr=state->sr[ea>>28];
  if (sr&UINT32_C(0x80000000)) return PPCVM_MMU_UNSUPPORTED; /* T=1: direct-store segment */
  *vsid=sr&UINT32_C(0x00ffffff);
  return PPCVM_MMU_OK;
}

/* 32-bit PowerPC hash: VSID[18:0] xor page index[15:0].
   SDR1 HTABORG contributes upper address bits, HTABMASK selects hash bits.
   This routine only calculates a candidate PTEG address. */
ppcvm_mmu_result ppcvm_mmu_pteg_address(const ppcvm_segment_state *state,
                                         uint32_t ea, int secondary,
                                         uint32_t *physical_address) {
  if (!state || !physical_address || (secondary!=0 && secondary!=1))
    return PPCVM_MMU_UNSUPPORTED;
  uint32_t vsid=0;
  if (ppcvm_mmu_segment_vsid(state,ea,&vsid)!=PPCVM_MMU_OK)
    return PPCVM_MMU_UNSUPPORTED;
  uint32_t hash=(vsid&UINT32_C(0x7ffff))^((ea>>12)&UINT32_C(0xffff));
  if (secondary) hash=~hash;
  uint32_t mask=(state->sdr1&UINT32_C(0x1ff))<<10;
  uint32_t index=hash&(mask|UINT32_C(0x3ff));
  *physical_address=(state->sdr1&UINT32_C(0xffff0000))|(index<<6);
  return PPCVM_MMU_OK;
}

/* First functional PTE scan. Physical RAM only, no protection/R/C handling. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte_access(const ppcvm_segment_state *state,
                                              const ppcvm_memory *ram, uint32_t ea,
                                              ppcvm_access access,
                                              uint32_t *physical_address) {
  if (!state || !ram || !physical_address ||
      (access!=PPCVM_ACCESS_INSTRUCTION && access!=PPCVM_ACCESS_DATA_READ &&
       access!=PPCVM_ACCESS_DATA_WRITE)) return PPCVM_MMU_UNSUPPORTED;
  uint32_t vsid=0;
  if (ppcvm_mmu_segment_vsid(state,ea,&vsid)!=PPCVM_MMU_OK)
    return PPCVM_MMU_UNSUPPORTED;
  uint32_t api=(ea>>22)&UINT32_C(0x3f);
  for (int secondary=0;secondary<=1;secondary++) {
    uint32_t pteg=0;
    if (ppcvm_mmu_pteg_address(state,ea,secondary,&pteg)!=PPCVM_MMU_OK)
      return PPCVM_MMU_UNSUPPORTED;
    for (uint32_t slot=0;slot<8;slot++) {
      uint32_t pte0=0,pte1=0;
      uint32_t addr=pteg+slot*8u;
      if (ppcvm_memory_read32be(ram,addr,&pte0)!=PPCVM_MEM_OK ||
          ppcvm_memory_read32be(ram,addr+4u,&pte1)!=PPCVM_MEM_OK)
        return PPCVM_MMU_UNSUPPORTED;
      if (!(pte0&UINT32_C(0x80000000))) continue;
      if (((pte0>>7)&UINT32_C(0xffffff))!=vsid ||
          ((pte0>>6)&1u)!=(uint32_t)secondary ||
          (pte0&UINT32_C(0x3f))!=api) continue;
      uint32_t pp=pte1&3u;
      if (pp==0u || (pp==1u && access==PPCVM_ACCESS_DATA_WRITE))
        return PPCVM_MMU_PROTECTION;
      *physical_address=(pte1&UINT32_C(0xfffff000))|(ea&UINT32_C(0xfff));
      return PPCVM_MMU_OK;
    }
  }
  return PPCVM_MMU_UNSUPPORTED;
}

/* Compatibility API: old callers request a data read. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte(const ppcvm_segment_state *state,
                                       const ppcvm_memory *ram, uint32_t ea,
                                       uint32_t *physical_address) {
  return ppcvm_mmu_lookup_pte_access(state,ram,ea,PPCVM_ACCESS_DATA_READ,
                                     physical_address);
}

/* A BAT protection violation is final; only a BAT miss falls back to PTE. */
ppcvm_mmu_result ppcvm_mmu_translate_combined(const ppcvm_bat_state *bat,
                                               const ppcvm_segment_state *segments,
                                               const ppcvm_memory *ram,
                                               uint32_t msr, uint32_t ea,
                                               ppcvm_access access, uint32_t *pa) {
  if (!bat || !segments || !ram || !pa) return PPCVM_MMU_UNSUPPORTED;
  ppcvm_mmu_result result=ppcvm_mmu_translate_bat(bat,msr,ea,access,pa);
  if (result!=PPCVM_MMU_UNSUPPORTED) return result;
  return ppcvm_mmu_lookup_pte_access(segments,ram,ea,access,pa);
}

/* Mutable page-table walk. Reuses the matching/permission model above but
   updates R/C only after the access has passed its protection check. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte_rc(const ppcvm_segment_state *state,
                                          ppcvm_memory *ram, uint32_t ea,
                                          ppcvm_access access, uint32_t *pa) {
  if (!state || !ram || !pa) return PPCVM_MMU_UNSUPPORTED;
  if (access!=PPCVM_ACCESS_INSTRUCTION && access!=PPCVM_ACCESS_DATA_READ &&
      access!=PPCVM_ACCESS_DATA_WRITE) return PPCVM_MMU_UNSUPPORTED;
  uint32_t vsid=0;
  if (ppcvm_mmu_segment_vsid(state,ea,&vsid)!=PPCVM_MMU_OK)
    return PPCVM_MMU_UNSUPPORTED;
  uint32_t api=(ea>>22)&63u;
  for (int secondary=0;secondary<=1;secondary++) {
    uint32_t pteg=0;
    if (ppcvm_mmu_pteg_address(state,ea,secondary,&pteg)!=PPCVM_MMU_OK)
      return PPCVM_MMU_UNSUPPORTED;
    for (uint32_t slot=0;slot<8;slot++) {
      uint32_t pte0=0,pte1=0,addr=pteg+slot*8u;
      if (ppcvm_memory_read32be(ram,addr,&pte0)!=PPCVM_MEM_OK ||
          ppcvm_memory_read32be(ram,addr+4u,&pte1)!=PPCVM_MEM_OK)
        return PPCVM_MMU_UNSUPPORTED;
      if (!(pte0&UINT32_C(0x80000000)) ||
          ((pte0>>7)&UINT32_C(0xffffff))!=vsid ||
          ((pte0>>6)&1u)!=(uint32_t)secondary ||
          (pte0&63u)!=api) continue;
      uint32_t pp=pte1&3u;
      if (pp==0u || (pp==1u && access==PPCVM_ACCESS_DATA_WRITE))
        return PPCVM_MMU_PROTECTION;
      uint32_t updated=pte1|UINT32_C(0x100);
      if (access==PPCVM_ACCESS_DATA_WRITE) updated|=UINT32_C(0x80);
      if (ppcvm_memory_write32be(ram,addr+4u,updated)!=PPCVM_MEM_OK)
        return PPCVM_MMU_UNSUPPORTED;
      *pa=(pte1&UINT32_C(0xfffff000))|(ea&UINT32_C(0xfff));
      return PPCVM_MMU_OK;
    }
  }
  return PPCVM_MMU_UNSUPPORTED;
}

/* 32-bit PowerPC hashed page-table protection matrix.
   Key 0: PP 00/01/10 permit RW, 11 read-only.
   Key 1: PP 00 denies, 01/11 read-only, 10 permits RW. */
ppcvm_mmu_result ppcvm_mmu_check_pte_permission(uint32_t key, uint32_t pp,
                                                 ppcvm_access access) {
  if (key>1u || pp>3u ||
      (access!=PPCVM_ACCESS_INSTRUCTION && access!=PPCVM_ACCESS_DATA_READ &&
       access!=PPCVM_ACCESS_DATA_WRITE)) return PPCVM_MMU_UNSUPPORTED;
  if (key==0u) {
    if (pp==3u && access==PPCVM_ACCESS_DATA_WRITE)
      return PPCVM_MMU_PROTECTION;
    return PPCVM_MMU_OK;
  }
  if (pp==0u || (pp!=2u && access==PPCVM_ACCESS_DATA_WRITE))
    return PPCVM_MMU_PROTECTION;
  return PPCVM_MMU_OK;
}
