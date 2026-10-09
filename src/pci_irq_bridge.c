#include "ppcvm/pci_irq_bridge.h"
#include <stddef.h>
int ppcvm_pci_irq_bridge_init(ppcvm_pci_irq_bridge *b,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *c,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,uint8_t source) {
  if(!b || !pci || !c || pin<1 || pin>4 || source>31) return -1;
  int found=0;
  for(size_t i=0;i<pci->count;i++)
    if(pci->slots[i].bus==bus && pci->slots[i].device==device &&
       pci->slots[i].function==function) {found=1;break;}
  if(!found) return -1;
  b->pci=pci;b->controller=c;b->bus=bus;b->device=device;
  b->function=function;b->pin=pin;b->source=source;b->asserted=0;
  return 0;
}
int ppcvm_pci_irq_bridge_set_level(ppcvm_pci_irq_bridge *b,int asserted) {
  if(!b || !b->controller || !b->pci) return -1;
  const uint32_t bit=UINT32_C(1)<<b->source;
  if(asserted) ppcvm_discovery_ii_assert_irq_low(b->controller,bit);
  else ppcvm_discovery_ii_clear_irq_low(b->controller,bit);
  b->asserted=(uint8_t)(asserted!=0);
  return 0;
}
