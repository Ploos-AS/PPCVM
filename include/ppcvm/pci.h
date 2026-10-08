#ifndef PPCVM_PCI_H
#define PPCVM_PCI_H
#include <stdint.h>
#include <stddef.h>
#define PPCVM_PCI_CONFIG_SIZE 256u
typedef struct {
  uint8_t config[PPCVM_PCI_CONFIG_SIZE];
} ppcvm_pci_device;
/* Generic PCI configuration header model; not a Pegasos II chipset map. */
void ppcvm_pci_device_init(ppcvm_pci_device *device, uint16_t vendor,
                           uint16_t product, uint8_t class_code,
                           uint8_t subclass, uint8_t revision);
int ppcvm_pci_read32(const ppcvm_pci_device *device, uint32_t offset,
                     uint32_t *value);
int ppcvm_pci_write32(ppcvm_pci_device *device, uint32_t offset,
                      uint32_t value);
#endif
