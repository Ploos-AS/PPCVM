#include "ppcvm/pci_irq_device.h"
#include <assert.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device config;
  ppcvm_discovery_ii c;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_irq_device a,b;
  uint32_t value=0;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&config,0x1234,0x5678,2,0,0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&config)==0);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&config)==0);
  ppcvm_discovery_ii_init(&c);
  ppcvm_discovery_ii_enable_irq_candidate(&c);
  c.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&c)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,3,0,1,0)==0);
  assert(ppcvm_pci_irq_device_init(&a,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_irq_device_init(&b,&matrix,0,3,0,1)==0);
  assert(ppcvm_pci_irq_device_write32(&a,0,0,1)==0);
  assert(ppcvm_pci_irq_device_write32(&b,0,0,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u);
  assert(ppcvm_pci_irq_device_read32(&a,0,4,&value)==0 && value==1u);
  assert(ppcvm_pci_irq_device_write32(&a,0,0,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==1u);
  assert(ppcvm_pci_irq_device_write32(&b,0,0,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&c)==0u);
  assert(ppcvm_pci_irq_device_write32(&a,0,4,1)==-1);
  assert(ppcvm_pci_irq_device_write32(&a,1,0,1)==-1);
  assert(ppcvm_pci_irq_device_write32(&a,0,0,2)==-1);
  return 0;
}
