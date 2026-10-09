#ifndef PPCVM_DISCOVERY_II_H
#define PPCVM_DISCOVERY_II_H
#include "ppcvm/bus.h"
#include "ppcvm/pci.h"
#include <stdint.h>
/* Experimental Discovery II controller. PCI config registers are opt-in;
 * all other unsupported offsets fail closed. This is not a complete
 * hardware-accurate MV64361 or Pegasos II memory map. */
/* Candidate MV643xx offsets from historical Linux headers and board examples.
 * Exposed only after explicit opt-in; MV64361 reset/mask semantics unverified. */
#define PPCVM_DISCOVERY_II_PCI0_CONFIG_ADDRESS_CANDIDATE UINT32_C(0x0cf8)
#define PPCVM_DISCOVERY_II_PCI0_CONFIG_DATA_CANDIDATE UINT32_C(0x0cfc)
#define PPCVM_DISCOVERY_II_PCI1_CONFIG_ADDRESS_CANDIDATE UINT32_C(0x0c78)
#define PPCVM_DISCOVERY_II_PCI1_CONFIG_DATA_CANDIDATE UINT32_C(0x0c7c)
typedef struct {
  ppcvm_pci_bus *pci[2];
  uint32_t config_address[2];
  uint8_t pci_config_enabled;
  uint32_t reset_count;
  uint32_t unsupported_reads;
  uint32_t unsupported_writes;
} ppcvm_discovery_ii;
/* Convert CPU-visible BE words to LE peripheral register words and back.
 * Involutive conversion; does not enable any undocumented register. */
uint32_t ppcvm_discovery_ii_swap32(uint32_t word);
/* Explicit opt-in experimental PCI config mode; not enabled by default. */
void ppcvm_discovery_ii_enable_pci_config(ppcvm_discovery_ii *controller,
    ppcvm_pci_bus *pci0,ppcvm_pci_bus *pci1);
void ppcvm_discovery_ii_init(ppcvm_discovery_ii *controller);
void ppcvm_discovery_ii_reset(ppcvm_discovery_ii *controller);
ppcvm_bus_result ppcvm_discovery_ii_map(ppcvm_bus *bus,
    ppcvm_discovery_ii *controller,uint32_t base,uint32_t size);
#endif
