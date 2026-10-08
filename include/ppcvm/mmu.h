#ifndef PPCVM_MMU_H
#define PPCVM_MMU_H
#include <stdint.h>
/* Initial G4 MMU interface. Only real-address mode is supported. */
typedef enum { PPCVM_MMU_OK=0, PPCVM_MMU_UNSUPPORTED=1 } ppcvm_mmu_result;
typedef enum { PPCVM_ACCESS_INSTRUCTION=0, PPCVM_ACCESS_DATA_READ=1, PPCVM_ACCESS_DATA_WRITE=2 } ppcvm_access;
ppcvm_mmu_result ppcvm_mmu_translate(uint32_t msr, uint32_t effective_address,
                                     ppcvm_access access, uint32_t *physical_address);
#endif
