#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device cfg;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device d;
  uint32_t value=99;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&cfg,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&cfg)==0);
  ppcvm_discovery_ii_init(&controller);
  ppcvm_discovery_ii_enable_irq_candidate(&controller);
  controller.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&d,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==0);
  assert(ppcvm_pci_event_device_read32(&d,0,4,&value)==0 && value==0);
  /* A disabled pending event does not assert the interrupt line. */
  assert(ppcvm_pci_event_device_write32(&d,0,12,1)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==1);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0);
  assert(ppcvm_pci_event_device_write32(&d,0,4,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1);
  /* Additional disabled event must remain pending. */
  assert(ppcvm_pci_event_device_write32(&d,0,12,2)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==3);
  assert(ppcvm_pci_event_device_write32(&d,0,8,1)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==2);
  assert(ppcvm_pci_event_device_write32(&d,0,4,2)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1);
  assert(ppcvm_pci_event_device_write32(&d,0,4,0)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==2);
  assert(ppcvm_pci_event_device_write32(&d,0,8,2)==0);
  assert(ppcvm_pci_event_device_read32(&d,0,0,&value)==0 && value==0);
  assert(ppcvm_pci_event_device_write32(&d,0,12,16)==-1);
  assert(ppcvm_pci_event_device_write32(&d,0,0,1)==-1);
  assert(ppcvm_pci_event_device_write32(&d,1,4,1)==-1);
  assert(ppcvm_pci_event_device_read32(&d,0,16,&value)==-1);
  return 0;
}
