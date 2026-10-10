#ifndef PPCVM_PCI_IRQ_DEVICE_H
#define PPCVM_PCI_IRQ_DEVICE_H
#include "ppcvm/pci_irq_matrix.h"
#include <stdint.h>
/* Diagnostic BAR0 device; registers are host-endian MMIO callback words.
 * Offset 0: control bit0 asserts INTx; offset 4: read-only status bit0.
 * Not a real Pegasos II peripheral. */
typedef struct {
  ppcvm_pci_irq_matrix *matrix;
  uint8_t bus,device,function,pin;
  uint32_t control;
} ppcvm_pci_irq_device;
int ppcvm_pci_irq_device_init(ppcvm_pci_irq_device *d,
    ppcvm_pci_irq_matrix *matrix,uint8_t bus,uint8_t device,
    uint8_t function,uint8_t pin);
int ppcvm_pci_irq_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value);
int ppcvm_pci_irq_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value);
#endif
