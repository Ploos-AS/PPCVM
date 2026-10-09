#include "ppcvm/pci_irq_shared.h"
#include <stddef.h>
int ppcvm_pci_irq_shared_init(ppcvm_pci_irq_shared *line,
    ppcvm_discovery_ii *controller,uint8_t source) {
  if(!line || !controller || source>31) return -1;
  line->controller=controller;line->source=source;
  line->owner_mask=0;line->asserted_mask=0;
  return 0;
}
int ppcvm_pci_irq_shared_register(ppcvm_pci_irq_shared *line,unsigned owner) {
  if(!line || !line->controller || owner>=PPCVM_PCI_IRQ_SHARED_MAX_OWNERS) return -1;
  uint16_t bit=(uint16_t)(1u<<owner);
  if(line->owner_mask & bit) return -1;
  line->owner_mask=(uint16_t)(line->owner_mask|bit);
  return 0;
}
int ppcvm_pci_irq_shared_set_level(ppcvm_pci_irq_shared *line,
    unsigned owner,int asserted) {
  if(!line || !line->controller || owner>=PPCVM_PCI_IRQ_SHARED_MAX_OWNERS) return -1;
  uint16_t bit=(uint16_t)(1u<<owner);
  if(!(line->owner_mask&bit)) return -1;
  if(asserted) line->asserted_mask=(uint16_t)(line->asserted_mask|bit);
  else line->asserted_mask=(uint16_t)(line->asserted_mask&~bit);
  uint32_t source_bit=UINT32_C(1)<<line->source;
  if(line->asserted_mask) ppcvm_discovery_ii_assert_irq_low(line->controller,source_bit);
  else ppcvm_discovery_ii_clear_irq_low(line->controller,source_bit);
  return 0;
}
