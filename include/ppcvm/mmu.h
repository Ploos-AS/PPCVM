#ifndef PPCVM_MMU_H
#define PPCVM_MMU_H
#include <stdint.h>
/* Initial G4 MMU interface; BAT-only translation is opt-in. */
typedef enum { PPCVM_MMU_OK=0, PPCVM_MMU_UNSUPPORTED=1 } ppcvm_mmu_result;
typedef enum { PPCVM_ACCESS_INSTRUCTION=0, PPCVM_ACCESS_DATA_READ=1, PPCVM_ACCESS_DATA_WRITE=2 } ppcvm_access;
ppcvm_mmu_result ppcvm_mmu_translate(uint32_t msr, uint32_t effective_address,
                                     ppcvm_access access, uint32_t *physical_address);
/* 60x/G4 BAT pairs; caller owns and initializes register state. */
typedef struct {
  uint32_t ibatu[4], ibatl[4];
  uint32_t dbatu[4], dbatl[4];
} ppcvm_bat_state;
/* Supports 128 KiB to 256 MiB BAT blocks, VS/VP and physical block mapping.
   PP=00 data access is rejected; remaining PP/WIMG semantics and page tables
   are not yet implemented. */
ppcvm_mmu_result ppcvm_mmu_translate_bat(const ppcvm_bat_state *state,
                                         uint32_t msr, uint32_t ea,
                                         ppcvm_access access, uint32_t *pa);
#endif
