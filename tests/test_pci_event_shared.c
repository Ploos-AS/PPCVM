#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device cfg;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device a,b;
  uint32_t value=0;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&cfg,0x1234,0x5681,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&cfg)==0);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&cfg)==0);
  ppcvm_discovery_ii_init(&controller);
  ppcvm_discovery_ii_enable_irq_candidate(&controller);
  controller.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,3,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&a,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_init(&b,&matrix,0,3,0,1)==0);
  /* Both devices assert the same source; acknowledgement is per-owner. */
  assert(ppcvm_pci_event_device_write32(&a,0,4,3)==0);
  assert(ppcvm_pci_event_device_write32(&b,0,4,1)==0);
  assert(ppcvm_pci_event_device_write32(&a,0,12,3)==0);
  assert(ppcvm_pci_event_device_write32(&b,0,12,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&a,0,8,1)==0);
  assert(a.pending==2u && b.pending==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&a,0,8,2)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&b,0,8,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  /* Disabled events accumulate; enabling later must assert without loss. */
  assert(ppcvm_pci_event_device_write32(&a,0,4,0)==0);
  assert(ppcvm_pci_event_device_write32(&a,0,12,5)==0);
  assert(a.pending==5u && ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_write32(&a,0,4,4)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&a,0,8,4)==0);
  assert(a.pending==1u && ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_write32(&a,0,4,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* Invalid writes must preserve both register state and asserted IRQ. */
  assert(ppcvm_pci_event_device_write32(&a,0,4,16)==-1);
  assert(ppcvm_pci_event_device_write32(&a,0,8,16)==-1);
  assert(ppcvm_pci_event_device_write32(&a,0,12,16)==-1);
  assert(ppcvm_pci_event_device_write32(&a,0,0,1)==-1);
  assert(ppcvm_pci_event_device_write32(&a,1,4,0)==-1);
  assert(ppcvm_pci_event_device_read32(&a,0,0,&value)==0 && value==1u);
  assert(ppcvm_pci_event_device_read32(&a,0,4,&value)==0 && value==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_write32(&a,0,8,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  return 0;
}
