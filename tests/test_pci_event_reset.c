#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device cfg_a, cfg_b;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device a,b;
  uint32_t value=99;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&cfg_a,0x1234,0x5681,2,0,1);
  ppcvm_pci_device_init(&cfg_b,0x1234,0x5682,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&cfg_a)==0);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&cfg_b)==0);
  ppcvm_discovery_ii_init(&controller);
  ppcvm_discovery_ii_enable_irq_candidate(&controller);
  controller.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,3,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&a,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_init(&b,&matrix,0,3,0,1)==0);
  assert(ppcvm_pci_event_device_write32(&a,0,4,3u)==0);
  assert(ppcvm_pci_event_device_write32(&b,0,4,1u)==0);
  assert(ppcvm_pci_event_device_write32(&a,0,12,3u)==0);
  assert(ppcvm_pci_event_device_write32(&b,0,12,1u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* Reset A only: B remains an active owner of the shared IRQ. */
  assert(ppcvm_pci_event_device_reset(&a)==0);
  assert(a.pending==0u && a.enable==0u);
  assert(b.pending==1u && b.enable==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_reset(&a)==0);
  assert(ppcvm_pci_event_device_read32(&a,0,0,&value)==0 && value==0u);
  assert(ppcvm_pci_event_device_read32(&a,0,4,&value)==0 && value==0u);
  assert(ppcvm_pci_event_device_reset(&b)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  /* Device is reusable immediately after reset. */
  assert(ppcvm_pci_event_device_write32(&a,0,12,4u)==0);
  assert(a.pending==4u && a.enable==0u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_write32(&a,0,4,4u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&a,0,8,4u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_reset(&a)==0);
  assert(a.pending==0u && a.enable==0u);
  /* Invalid input must not mutate a valid device. */
  assert(ppcvm_pci_event_device_reset(0)==-1);
  assert(a.pending==0u && a.enable==0u);
  return 0;
}
