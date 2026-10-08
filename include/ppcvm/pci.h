#ifndef PPCVM_PCI_H
#define PPCVM_PCI_H
#include <stdint.h>
#include <stddef.h>
#define PPCVM_PCI_CONFIG_SIZE 256u
typedef struct {
  uint8_t config[PPCVM_PCI_CONFIG_SIZE];
  uint32_t bar_size[6];
  uint8_t bar_probe[6];
  uint8_t bar_io[6];
  uint8_t bar64[6];
  uint64_t bar64_size[6];
} ppcvm_pci_device;
/* Generic PCI configuration header model; not a Pegasos II chipset map. */
void ppcvm_pci_device_init(ppcvm_pci_device *device, uint16_t vendor,
                           uint16_t product, uint8_t class_code,
                           uint8_t subclass, uint8_t revision);
/* Configure one 32-bit non-prefetchable memory BAR; size power-of-two >=16. */
int ppcvm_pci_set_mem_bar32(ppcvm_pci_device *device, unsigned index,
                             uint32_t size, uint32_t base);
/* Configure one 32-bit I/O BAR; size power-of-two >=4, aligned base. */
int ppcvm_pci_set_io_bar32(ppcvm_pci_device *device, unsigned index,
                            uint32_t size, uint32_t base);
/* Configure a 64-bit memory BAR occupying index and index+1. */
int ppcvm_pci_set_mem_bar64(ppcvm_pci_device *device, unsigned index,
                             uint64_t size, uint64_t base);
int ppcvm_pci_read32(const ppcvm_pci_device *device, uint32_t offset,
                     uint32_t *value);
int ppcvm_pci_write32(ppcvm_pci_device *device, uint32_t offset,
                      uint32_t value);
/* Small standalone BDF registry; no physical host bridge attached. */
#define PPCVM_PCI_MAX_DEVICES 16u
typedef struct {
  uint8_t bus, device, function;
  ppcvm_pci_device config;
  void *mmio_context;
  int (*mmio_read32)(void *context, uint8_t bar, uint64_t offset, uint32_t *value);
  int (*mmio_write32)(void *context, uint8_t bar, uint64_t offset, uint32_t value);
} ppcvm_pci_slot;
typedef struct {
  ppcvm_pci_slot slots[PPCVM_PCI_MAX_DEVICES];
  size_t count;
} ppcvm_pci_bus;
void ppcvm_pci_bus_init(ppcvm_pci_bus *bus);
int ppcvm_pci_bus_add(ppcvm_pci_bus *bus, uint8_t bus_number,
    uint8_t device, uint8_t function, const ppcvm_pci_device *config);
int ppcvm_pci_bus_read32(const ppcvm_pci_bus *bus, uint8_t bus_number,
    uint8_t device, uint8_t function, uint32_t offset, uint32_t *value);
int ppcvm_pci_bus_write32(ppcvm_pci_bus *bus, uint8_t bus_number,
    uint8_t device, uint8_t function, uint32_t offset, uint32_t value);
/* Decode configured memory BAR address ranges (no guest bus routing yet).
   Returns 0 for exactly one match, 1 for no match, -1 for ambiguity/error. */
typedef struct {
  uint8_t bus, device, function, bar_index;
  uint64_t offset;
} ppcvm_pci_bar_hit;
int ppcvm_pci_bus_decode_memory(const ppcvm_pci_bus *bus, uint64_t address,
                                 ppcvm_pci_bar_hit *hit);
/* Decode configured I/O BARs; PCI Command bit 0 must be enabled.
   Returns 0 for one match, 1 for no match, -1 for overlap/error. */
int ppcvm_pci_bus_decode_io(const ppcvm_pci_bus *bus, uint32_t address,
                             ppcvm_pci_bar_hit *hit);
/* Optional host-side 32-bit MMIO dispatch; guest CPU bus is not wired. */
int ppcvm_pci_bus_set_mmio(ppcvm_pci_bus *bus, uint8_t bus_number,
    uint8_t device, uint8_t function, void *context,
    int (*read32)(void *, uint8_t, uint64_t, uint32_t *),
    int (*write32)(void *, uint8_t, uint64_t, uint32_t));
int ppcvm_pci_bus_mmio_read32(ppcvm_pci_bus *bus, uint64_t address,
                               uint32_t *value);
int ppcvm_pci_bus_mmio_write32(ppcvm_pci_bus *bus, uint64_t address,
                                uint32_t value);
#endif
