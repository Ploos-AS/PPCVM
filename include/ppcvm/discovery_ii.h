#ifndef PPCVM_DISCOVERY_II_H
#define PPCVM_DISCOVERY_II_H
#include "ppcvm/bus.h"
#include <stdint.h>
/* M4.1 controller skeleton: deliberately NO hardware register offsets yet.
 * A mapped region rejects accesses until verified register semantics exist.
 * This must not be confused with the synthetic PCI config bridge. */
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
