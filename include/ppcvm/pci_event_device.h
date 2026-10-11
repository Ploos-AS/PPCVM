#ifndef PPCVM_PCI_EVENT_DEVICE_H
#define PPCVM_PCI_EVENT_DEVICE_H
#include "ppcvm/pci_irq_matrix.h"
#include <stdint.h>
#include <stddef.h>
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
/* Reset an initialized device, releasing only its IRQ matrix ownership. */
int ppcvm_pci_event_device_reset(ppcvm_pci_event_device *d);
#define PPCVM_PCI_EVENT_SNAPSHOT_VERSION UINT32_C(1)
typedef struct {
  uint32_t version;
  uint32_t pending;
  uint32_t enable;
} ppcvm_pci_event_snapshot;
/* Snapshot is pointer-free; restore validates before changing IRQ state. */
int ppcvm_pci_event_device_save(const ppcvm_pci_event_device *d,
    ppcvm_pci_event_snapshot *snapshot);
int ppcvm_pci_event_device_restore(ppcvm_pci_event_device *d,
    const ppcvm_pci_event_snapshot *snapshot);
#define PPCVM_PCI_EVENT_WIRE_SIZE 16u
/* PEVT + BE32 version + BE32 pending + BE32 enable. */
int ppcvm_pci_event_snapshot_encode(const ppcvm_pci_event_snapshot *snapshot,
    uint8_t *out,size_t size);
int ppcvm_pci_event_snapshot_decode(ppcvm_pci_event_snapshot *snapshot,
    const uint8_t *data,size_t size);
/* Persist exactly one PEVT frame. Loading validates before restore. */
int ppcvm_pci_event_device_save_file(const ppcvm_pci_event_device *d,
    const char *path);
int ppcvm_pci_event_device_load_file(ppcvm_pci_event_device *d,
    const char *path);
int ppcvm_pci_event_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value);
int ppcvm_pci_event_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value);
#endif
