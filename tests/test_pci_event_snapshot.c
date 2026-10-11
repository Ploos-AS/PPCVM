#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus pci;
  ppcvm_pci_device cfg_a,cfg_b;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device a,b,orphan;
  ppcvm_pci_event_snapshot saved,bad,decoded;
  uint8_t wire[PPCVM_PCI_EVENT_WIRE_SIZE];
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
  assert(ppcvm_pci_event_device_write32(&a,0,12,3u)==0);
  assert(ppcvm_pci_event_device_write32(&a,0,4,1u)==0);
  assert(ppcvm_pci_event_device_save(&a,&saved)==0);
  assert(saved.version==PPCVM_PCI_EVENT_SNAPSHOT_VERSION);
  assert(saved.pending==3u && saved.enable==1u);
  assert(ppcvm_pci_event_snapshot_encode(&saved,wire,sizeof(wire))==0);
  assert(ppcvm_pci_event_snapshot_decode(&decoded,wire,sizeof(wire))==0);
  assert(decoded.pending==3u && decoded.enable==1u);
  assert(ppcvm_pci_event_device_reset(&a)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_restore(&a,&decoded)==0);
  assert(a.pending==3u && a.enable==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* A masked pending event survives snapshot and later unmask. */
  assert(ppcvm_pci_event_device_write32(&a,0,8,1u)==0);
  assert(a.pending==2u && ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(ppcvm_pci_event_device_write32(&a,0,4,2u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_reset(&a)==0);
  /* Restore of A must not release B's shared IRQ ownership. */
  assert(ppcvm_pci_event_device_write32(&b,0,4,1u)==0);
  assert(ppcvm_pci_event_device_write32(&b,0,12,1u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_restore(&a,&saved)==0);
  assert(ppcvm_pci_event_device_reset(&a)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* Invalid snapshots must not mutate registers or IRQ state. */
  bad=saved;bad.version++;
  assert(ppcvm_pci_event_device_restore(&a,&bad)==-1);
  bad=saved;bad.pending=16u;
  assert(ppcvm_pci_event_device_restore(&a,&bad)==-1);
  bad=saved;bad.enable=16u;
  assert(ppcvm_pci_event_device_restore(&a,&bad)==-1);
  assert(a.pending==0u && a.enable==0u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_init(&orphan,&matrix,0,7,0,1)==0);
  assert(ppcvm_pci_event_device_restore(&orphan,&saved)==-1);
  assert(orphan.pending==0u && orphan.enable==0u);
  assert(ppcvm_pci_event_device_restore(&a,0)==-1);
  assert(ppcvm_pci_event_device_save(0,&saved)==-1);
  assert(ppcvm_pci_event_device_write32(&b,0,8,1u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  return 0;
}
