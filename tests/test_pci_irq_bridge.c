#include "ppcvm/pci_irq_bridge.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device device;
  ppcvm_discovery_ii c;
  ppcvm_pci_irq_bridge a,b;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  ppcvm_discovery_ii_init(&c);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  assert(ppcvm_pci_irq_bridge_init(&a,&pci,&c,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_bridge_init(&b,&pci,&c,0,2,0,2,1)==0);
  assert(ppcvm_pci_irq_bridge_init(&b,&pci,&c,0,3,0,1,1)==-1);
  assert(ppcvm_pci_irq_bridge_init(&b,&pci,&c,0,2,0,0,1)==-1);
  assert(ppcvm_pci_irq_bridge_init(&b,&pci,&c,0,2,0,2,32)==-1);
  assert(ppcvm_pci_irq_bridge_init(&b,&pci,&c,0,2,0,2,1)==0);
  c.irq_cpu0_mask_low=3u;
  assert(ppcvm_pci_irq_bridge_set_level(&a,1)==0);
  assert(ppcvm_pci_irq_bridge_set_level(&b,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==3u);
  assert(ppcvm_pci_irq_bridge_set_level(&a,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==2u);
  assert(ppcvm_pci_irq_bridge_set_level(&b,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0);
  return 0;
}
