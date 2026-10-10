#include "ppcvm/pegasos2.h"
#include "ppcvm/pci_irq_device.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  ppcvm_pci_bus pci;
  ppcvm_pci_device cfg;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_irq_device device;
  ppcvm_pci_mmio_aperture aperture;
  uint32_t value=0;
  assert(ppcvm_pegasos2_init(&m,65536u)==0);
  ppcvm_discovery_ii_enable_irq_candidate(&m.discovery_ii);
  m.discovery_ii.irq_cpu0_mask_low=1u;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&cfg,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_set_mem_bar32(&cfg,0,16u,0x30000u)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&cfg)==0);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4u,2u)==0);
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&m.discovery_ii)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_device_init(&device,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&device,
      ppcvm_pci_irq_device_read32,ppcvm_pci_irq_device_write32)==0);
  assert(ppcvm_pci_map_mmio_aperture(&m.bus,&aperture,&pci,0x30000u,16u)==PPCVM_BUS_OK);
  assert(ppcvm_pci_bus_mmio_write32(&pci,0x30000u,1u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==1u);
  assert(ppcvm_pci_bus_mmio_read32(&pci,0x30004u,&value)==0 && value==1u);
  assert(ppcvm_pci_bus_mmio_write32(&pci,0x30000u,0u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==0u);
  assert(ppcvm_pci_bus_mmio_write32(&pci,0x30004u,1u)==-1);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
