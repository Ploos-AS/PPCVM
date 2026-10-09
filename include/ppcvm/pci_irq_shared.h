#ifndef PPCVM_PCI_IRQ_SHARED_H
#define PPCVM_PCI_IRQ_SHARED_H
#include "ppcvm/pci_irq_bridge.h"
#include <stdint.h>
#define PPCVM_PCI_IRQ_SHARED_MAX_OWNERS 16u
/* Explicit synthetic wired-OR for bridges sharing one controller source. */
typedef struct {
  ppcvm_discovery_ii *controller;
  uint8_t source;
  uint16_t owner_mask;
  uint16_t asserted_mask;
} ppcvm_pci_irq_shared;
int ppcvm_pci_irq_shared_init(ppcvm_pci_irq_shared *line,
    ppcvm_discovery_ii *controller,uint8_t source);
/* Each owner must use a distinct index (0..15). */
int ppcvm_pci_irq_shared_register(ppcvm_pci_irq_shared *line,unsigned owner);
int ppcvm_pci_irq_shared_set_level(ppcvm_pci_irq_shared *line,
    unsigned owner,int asserted);
#endif
