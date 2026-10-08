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
/* Opt-in PTE lookup with simplified PP protection (00 none, 01 read,
   10/11 read-write). Does not model segment keys or R/C updates. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte_access(const ppcvm_segment_state *state,
                                              const ppcvm_memory *ram,
                                              uint32_t ea, ppcvm_access access,
                                              uint32_t *physical_address);
/* PowerPC 32-bit hashed PTE protection helper.
   key is the segment protection key (0 or 1); pp is PTE PP (0..3).
   Instruction fetch uses the read permission path in this prototype. */
ppcvm_mmu_result ppcvm_mmu_check_pte_permission(uint32_t key, uint32_t pp,
                                                 ppcvm_access access);
/* Segment key from SR Ks/Kp (bits 30/29) selected by MSR[PR].
   This helper is opt-in until the key-aware walker is integrated. */
ppcvm_mmu_result ppcvm_mmu_segment_key(const ppcvm_segment_state *state,
                                        uint32_t msr, uint32_t ea,
                                        uint32_t *key);
/* Key-aware PTE walk; legacy lookup remains unchanged for compatibility.
   R/C updates are not performed by this read-only variant. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte_keyed(const ppcvm_segment_state *state,
                                             const ppcvm_memory *ram,
                                             uint32_t msr, uint32_t ea,
                                             ppcvm_access access, uint32_t *pa);
/* Opt-in mutable PTE walk: set R (bit 8) on reads/fetches and R+C
   (bits 8,7) on writes after a permitted translation. CPU stepping uses this after successful bus accesses; segment keys are\n   not yet wired into the CPU translation path. */
ppcvm_mmu_result ppcvm_mmu_lookup_pte_rc(const ppcvm_segment_state *state,
                                          ppcvm_memory *ram, uint32_t ea,
                                          ppcvm_access access, uint32_t *pa);
/* Combined opt-in translation: real mode, BAT, then hashed PTE fallback.
   PTE scan currently requires a flat physical RAM backing store. */
ppcvm_mmu_result ppcvm_mmu_translate_combined(const ppcvm_bat_state *bat,
                                               const ppcvm_segment_state *segments,
                                               const ppcvm_memory *ram,
                                               uint32_t msr, uint32_t ea,
                                               ppcvm_access access, uint32_t *pa);
/* Supports 128 KiB to 256 MiB BAT blocks, VS/VP and physical block mapping.
   PP=00 denies data and PP=01 is read-only in the current simplified model.
   Full privilege/key semantics, WIMG and page tables are not implemented. */
ppcvm_mmu_result ppcvm_mmu_translate_bat(const ppcvm_bat_state *state,
                                         uint32_t msr, uint32_t ea,
                                         ppcvm_access access, uint32_t *pa);
#endif
