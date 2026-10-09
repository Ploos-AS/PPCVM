#include "ppcvm/pci_irq_router.h"
#include <assert.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device d;
  ppcvm_discovery_ii c;
  ppcvm_pci_irq_router r;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&d,0x1234,0x5678,2,0,0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&d)==0);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&d)==0);
  ppcvm_discovery_ii_init(&c);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  c.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_router_init(&r,&pci,&c,0)==0);
  assert(ppcvm_pci_irq_router_add(&r,0,9,0,1)==-1);
  assert(ppcvm_pci_irq_router_add(&r,0,2,0,0)==-1);
  assert(ppcvm_pci_irq_router_add(&r,0,2,0,5)==-1);
  assert(ppcvm_pci_irq_router_add(&r,0,2,0,1)==0);
  assert(ppcvm_pci_irq_router_add(&r,0,2,0,1)==-1);
  assert(ppcvm_pci_irq_router_add(&r,0,3,0,1)==0);
  assert(ppcvm_pci_irq_router_set_level(&r,0,9,0,1,1)==-1);
  assert(ppcvm_pci_irq_router_set_level(&r,0,2,0,2,1)==-1);
  assert(ppcvm_pci_irq_router_set_level(&r,0,2,0,1,1)==0);
  assert(ppcvm_pci_irq_router_set_level(&r,0,3,0,1,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u);
  assert(ppcvm_pci_irq_router_set_level(&r,0,2,0,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u);
  assert(ppcvm_pci_irq_router_set_level(&r,0,3,0,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0u);
  return 0;
}
