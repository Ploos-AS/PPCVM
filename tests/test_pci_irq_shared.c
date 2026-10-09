#include "ppcvm/pci_irq_shared.h"
#include <assert.h>
int main(void) {
  ppcvm_discovery_ii c;
  ppcvm_pci_irq_shared shared,other;
  ppcvm_discovery_ii_init(&c);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  c.irq_cpu0_mask_low=3u;
  assert(ppcvm_pci_irq_shared_init(&shared,&c,0)==0);
  assert(ppcvm_pci_irq_shared_init(&other,&c,1)==0);
  assert(ppcvm_pci_irq_shared_register(&shared,0)==0);
  assert(ppcvm_pci_irq_shared_register(&shared,1)==0);
  assert(ppcvm_pci_irq_shared_register(&shared,1)==-1);
  assert(ppcvm_pci_irq_shared_set_level(&shared,2,1)==-1);
  assert(ppcvm_pci_irq_shared_register(&other,0)==0);
  assert(ppcvm_pci_irq_shared_set_level(&shared,0,1)==0);
  assert(ppcvm_pci_irq_shared_set_level(&shared,1,1)==0);
  assert(ppcvm_pci_irq_shared_set_level(&other,0,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==3u);
  assert(ppcvm_pci_irq_shared_set_level(&shared,0,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==3u);
  assert(ppcvm_pci_irq_shared_set_level(&shared,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==2u);
  assert(ppcvm_pci_irq_shared_set_level(&other,0,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0u);
  return 0;
}
