#ifndef PPCVM_PCI_IRQ_BRIDGE_H
#define PPCVM_PCI_IRQ_BRIDGE_H
#include "ppcvm/discovery_ii.h"
#include "ppcvm/pci.h"
#include <stdint.h>
/* Synthetic, opt-in PCI INTx routing; NOT Pegasos II board wiring. */
typedef struct {
  ppcvm_pci_bus *pci;
  ppcvm_discovery_ii *controller;
  uint8_t bus,device,function,pin;
  uint8_t source;
  uint8_t asserted;
} ppcvm_pci_irq_bridge;
/* Returns -1 for invalid slot, pin (1..4), or source (0..31). */
int ppcvm_pci_irq_bridge_init(ppcvm_pci_irq_bridge *bridge,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *controller,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,uint8_t source);
/* Level assertion/deassertion from an emulated PCI device. */
int ppcvm_pci_irq_bridge_set_level(ppcvm_pci_irq_bridge *bridge,int asserted);
#endif
