#ifndef PPCVM_PCI_EVENT_DEVICE_H
#define PPCVM_PCI_EVENT_DEVICE_H
#include "ppcvm/pci_irq_matrix.h"
#include <stdint.h>
/* Synthetic event-driven PCI BAR0 model; not a physical Pegasos II device.
 * 0x00 PENDING: read-only pending event bits
 * 0x04 ENABLE: read/write interrupt enable bits
 * 0x08 ACK: write-one-to-clear pending events; reads return zero
 * 0x0c RAISE: write-one-to-set pending events; reads return zero
 * Valid event bits: 0..3. INTx level = (PENDING & ENABLE) != 0.
 * MMIO callbacks use host-order words; the CPU bus handles byte swapping. */
typedef struct {
  ppcvm_pci_irq_matrix *matrix;
  uint8_t bus, device, function, pin;
  uint32_t pending, enable;
} ppcvm_pci_event_device;
int ppcvm_pci_event_device_init(ppcvm_pci_event_device *d,
    ppcvm_pci_irq_matrix *matrix,uint8_t bus,uint8_t device,
    uint8_t function,uint8_t pin);
int ppcvm_pci_event_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value);
int ppcvm_pci_event_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value);
#endif
