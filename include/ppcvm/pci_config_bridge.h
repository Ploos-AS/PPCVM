#ifndef PPCVM_PCI_CONFIG_BRIDGE_H
#define PPCVM_PCI_CONFIG_BRIDGE_H
#include "ppcvm/pci.h"

/* Prototype, synthetic hostbridge: two big-endian 32-bit MMIO registers.
 * Offset 0: address (bit31 enable; bus[23:16], device[15:11],
 *           function[10:8], config register[7:2]).
 * Offset 4: config data. Absent devices return 0xffffffff.
 * This is NOT the Pegasos II/MV64361 hardware register map. */
typedef struct {
  ppcvm_pci_bus *pci;
  uint32_t address;
} ppcvm_pci_config_bridge;
ppcvm_bus_result ppcvm_pci_map_config_bridge(ppcvm_bus *cpu_bus,
    ppcvm_pci_config_bridge *bridge,ppcvm_pci_bus *pci,uint32_t base);
#endif
