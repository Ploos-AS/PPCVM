#ifndef PPCVM_MMU_H
#define PPCVM_MMU_H
#include <stdint.h>
#include "ppcvm/memory.h"
/* Initial G4 MMU interface; BAT-only translation is opt-in. */
typedef enum { PPCVM_MMU_OK=0, PPCVM_MMU_UNSUPPORTED=1, PPCVM_MMU_PROTECTION=2 } ppcvm_mmu_result;
typedef enum { PPCVM_ACCESS_INSTRUCTION=0, PPCVM_ACCESS_DATA_READ=1, PPCVM_ACCESS_DATA_WRITE=2 } ppcvm_access;
ppcvm_mmu_result ppcvm_mmu_translate(uint32_t msr, uint32_t effective_address,
                                     ppcvm_access access, uint32_t *physical_address);
/* 60x/G4 BAT pairs; caller owns and initializes register state. */
typedef struct {
  uint32_t ibatu[4], ibatl[4];
  uint32_t dbatu[4], dbatl[4];
} ppcvm_bat_state;
/* Segment register state for future hashed page-table translation.
   The top four bits of an effective address select one of 16 segments.
   VSID occupies the low 24 bits of each segment register. */
typedef struct {
  uint32_t sr[16];
  uint32_t sdr1;
} ppcvm_segment_state;
ppcvm_mmu_result ppcvm_mmu_segment_vsid(const ppcvm_segment_state *state,
                                         uint32_t ea, uint32_t *vsid);
/* Initial 32-bit hashed page table address calculation (PTEG address only).
   No PTE scanning, page permissions, or TLB behavior is implied. */
ppcvm_mmu_result ppcvm_mmu_pteg_address(const ppcvm_segment_state *state,
                                         uint32_t ea, int secondary,
                                         uint32_t *physical_address);
/* Prototype PTEG scan over flat physical RAM: V/VSID/H/API match only.
   Page permissions, R/C bits, memory attributes and TLB are not handled. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte(const ppcvm_segment_state *state,
                                       const ppcvm_memory *ram, uint32_t ea,
                                       uint32_t *physical_address);
/* Supports 128 KiB to 256 MiB BAT blocks, VS/VP and physical block mapping.
   PP=00 denies data and PP=01 is read-only in the current simplified model.
   Full privilege/key semantics, WIMG and page tables are not implemented. */
ppcvm_mmu_result ppcvm_mmu_translate_bat(const ppcvm_bat_state *state,
                                         uint32_t msr, uint32_t ea,
                                         ppcvm_access access, uint32_t *pa);
#endif
