#ifndef PPCVM_PCI_IRQ_ROUTER_H
#define PPCVM_PCI_IRQ_ROUTER_H
#include "ppcvm/pci_irq_shared.h"
/* Experimental one-source router: BDF+pin uniquely identifies each owner.
 * Does not represent a validated Pegasos II PCI interrupt swizzle. */
typedef struct {
  ppcvm_pci_bus *pci;
  ppcvm_pci_irq_shared line;
  struct {uint8_t bus,device,function,pin;} owners[PPCVM_PCI_IRQ_SHARED_MAX_OWNERS];
  unsigned count;
} ppcvm_pci_irq_router;
int ppcvm_pci_irq_router_init(ppcvm_pci_irq_router *r,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *controller,uint8_t source);
int ppcvm_pci_irq_router_add(ppcvm_pci_irq_router *r,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin);
int ppcvm_pci_irq_router_set_level(ppcvm_pci_irq_router *r,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,int asserted);
#endif
