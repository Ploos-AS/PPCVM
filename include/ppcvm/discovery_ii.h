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
/* MV64340-family register offsets from historical Linux mv643xx.h;
 * applicability to MV64361/Pegasos II remains provisional. */
#define PPCVM_DISCOVERY_II_IRQ_CAUSE_LOW_CANDIDATE UINT32_C(0x004)
#define PPCVM_DISCOVERY_II_IRQ_CPU0_MASK_LOW_CANDIDATE UINT32_C(0x014)
typedef struct {
  ppcvm_pci_bus *pci[2];
  uint32_t config_address[2];
  uint8_t pci_config_enabled;
  uint8_t irq_candidate_enabled;
  uint32_t irq_asserted_low;
  uint32_t irq_cpu0_mask_low;
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
/* Experimental CPU0 low-bank IRQ path, explicitly opt-in. */
void ppcvm_discovery_ii_enable_irq_candidate(ppcvm_discovery_ii *controller);
void ppcvm_discovery_ii_assert_irq_low(ppcvm_discovery_ii *controller,uint32_t bits);
void ppcvm_discovery_ii_clear_irq_low(ppcvm_discovery_ii *controller,uint32_t bits);
uint32_t ppcvm_discovery_ii_active_irq_low(const ppcvm_discovery_ii *controller);
void ppcvm_discovery_ii_init(ppcvm_discovery_ii *controller);
void ppcvm_discovery_ii_reset(ppcvm_discovery_ii *controller);
ppcvm_bus_result ppcvm_discovery_ii_map(ppcvm_bus *bus,
    ppcvm_discovery_ii *controller,uint32_t base,uint32_t size);
#endif
