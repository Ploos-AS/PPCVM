#ifndef PPCVM_DISCOVERY_II_H
#define PPCVM_DISCOVERY_II_H
#include "ppcvm/bus.h"
#include <stdint.h>
/* M4.1 controller skeleton: deliberately NO hardware register offsets yet.
 * A mapped region rejects accesses until verified register semantics exist.
 * This must not be confused with the synthetic PCI config bridge. */
/* Candidate MV643xx PCI config offsets from historical Linux headers.
 * These are NOT enabled as hardware registers until MV64361 is validated. */
#define PPCVM_DISCOVERY_II_PCI0_CONFIG_ADDRESS_CANDIDATE UINT32_C(0x0cf8)
#define PPCVM_DISCOVERY_II_PCI0_CONFIG_DATA_CANDIDATE UINT32_C(0x0cfc)
#define PPCVM_DISCOVERY_II_PCI1_CONFIG_ADDRESS_CANDIDATE UINT32_C(0x0c78)
#define PPCVM_DISCOVERY_II_PCI1_CONFIG_DATA_CANDIDATE UINT32_C(0x0c7c)
typedef struct {
  uint32_t reset_count;
  uint32_t unsupported_reads;
  uint32_t unsupported_writes;
} ppcvm_discovery_ii;
void ppcvm_discovery_ii_init(ppcvm_discovery_ii *controller);
void ppcvm_discovery_ii_reset(ppcvm_discovery_ii *controller);
ppcvm_bus_result ppcvm_discovery_ii_map(ppcvm_bus *bus,
    ppcvm_discovery_ii *controller,uint32_t base,uint32_t size);
#endif
