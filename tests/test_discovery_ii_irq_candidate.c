#include "ppcvm/discovery_ii.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_discovery_ii c;
  ppcvm_bus bus;
  uint32_t value=0;
  ppcvm_discovery_ii_init(&c);
  ppcvm_bus_init(&bus);
  assert(ppcvm_discovery_ii_map(&bus,&c,0x10000u,0x100u)==PPCVM_BUS_OK);
  ppcvm_discovery_ii_assert_irq_low(&c,3u);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0);
  assert(ppcvm_bus_read32be(&bus,0x10004u,&value)==PPCVM_BUS_UNMAPPED);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  assert(ppcvm_bus_read32be(&bus,0x10004u,&value)==PPCVM_BUS_OK);
  assert(value==ppcvm_discovery_ii_swap32(3u));
  assert(ppcvm_bus_write32be(&bus,0x10014u,ppcvm_discovery_ii_swap32(1u))==PPCVM_BUS_OK);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u);
  assert(ppcvm_bus_read32be(&bus,0x10014u,&value)==PPCVM_BUS_OK);
  assert(value==ppcvm_discovery_ii_swap32(1u));
  assert(ppcvm_bus_write32be(&bus,0x10004u,0u)==PPCVM_BUS_UNMAPPED);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u); /* cause read-only */
  ppcvm_discovery_ii_clear_irq_low(&c,1u);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0u);
  ppcvm_discovery_ii_reset(&c);
  assert(c.irq_asserted_low==0 && c.irq_cpu0_mask_low==0);
  return 0;
}
