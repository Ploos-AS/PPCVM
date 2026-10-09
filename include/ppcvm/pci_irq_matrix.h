#ifndef PPCVM_PCI_IRQ_MATRIX_H
#define PPCVM_PCI_IRQ_MATRIX_H
#include "ppcvm/pci_irq_router.h"
#define PPCVM_PCI_IRQ_MATRIX_SOURCES 32u
/* Each source has an independent wired-OR owner set. The caller supplies
 * the source mapping; no physical Pegasos II swizzle is assumed. */
typedef struct {
  ppcvm_pci_bus *pci;
  ppcvm_discovery_ii *controller;
  ppcvm_pci_irq_router sources[PPCVM_PCI_IRQ_MATRIX_SOURCES];
} ppcvm_pci_irq_matrix;
int ppcvm_pci_irq_matrix_init(ppcvm_pci_irq_matrix *matrix,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *controller);
int ppcvm_pci_irq_matrix_add(ppcvm_pci_irq_matrix *matrix,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,uint8_t source);
int ppcvm_pci_irq_matrix_set_level(ppcvm_pci_irq_matrix *matrix,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,int asserted);
#endif
