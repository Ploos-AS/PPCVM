#include "ppcvm/pci_irq_matrix.h"
#include <assert.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device d;
  ppcvm_discovery_ii c;
  ppcvm_pci_irq_matrix m;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&d,0x1234,0x5678,2,0,0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&d)==0);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&d)==0);
  assert(ppcvm_pci_bus_add(&pci,0,4,0,&d)==0);
  ppcvm_discovery_ii_init(&c);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  c.irq_cpu0_mask_low=3u;
  assert(ppcvm_pci_irq_matrix_init(&m,&pci,&c)==0);
  assert(ppcvm_pci_irq_matrix_add(&m,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&m,0,3,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&m,0,4,0,1,1)==0);
  assert(ppcvm_pci_irq_matrix_add(&m,0,2,0,1,1)==-1);
  assert(ppcvm_pci_irq_matrix_add(&m,0,5,0,1,2)==-1);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,5,0,1,1)==-1);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,2,0,1,1)==0);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,3,0,1,1)==0);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,4,0,1,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==3u);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,2,0,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==3u);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,3,0,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==2u);
  assert(ppcvm_pci_irq_matrix_set_level(&m,0,4,0,1,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0u);
  return 0;
}
