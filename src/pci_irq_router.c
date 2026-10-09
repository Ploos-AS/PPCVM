#include "ppcvm/pci_irq_router.h"
#include <stddef.h>
static int slot_exists(const ppcvm_pci_bus *pci,uint8_t bus,uint8_t device,uint8_t function) {
  for(size_t i=0;i<pci->count;i++)
    if(pci->slots[i].bus==bus && pci->slots[i].device==device &&
       pci->slots[i].function==function) return 1;
  return 0;
}
int ppcvm_pci_irq_router_init(ppcvm_pci_irq_router *r,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *controller,uint8_t source) {
  if(!r || !pci || ppcvm_pci_irq_shared_init(&r->line,controller,source)!=0) return -1;
  r->pci=pci;r->count=0;
  return 0;
}
int ppcvm_pci_irq_router_add(ppcvm_pci_irq_router *r,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin) {
  if(!r || !r->pci || pin<1 || pin>4 || r->count>=PPCVM_PCI_IRQ_SHARED_MAX_OWNERS ||
     !slot_exists(r->pci,bus,device,function)) return -1;
  for(unsigned i=0;i<r->count;i++)
    if(r->owners[i].bus==bus && r->owners[i].device==device &&
       r->owners[i].function==function && r->owners[i].pin==pin) return -1;
  unsigned owner=r->count;
  if(ppcvm_pci_irq_shared_register(&r->line,owner)!=0) return -1;
  r->owners[owner].bus=bus;r->owners[owner].device=device;
  r->owners[owner].function=function;r->owners[owner].pin=pin;
  r->count++;
  return 0;
}
int ppcvm_pci_irq_router_set_level(ppcvm_pci_irq_router *r,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,int asserted) {
  if(!r || !r->pci) return -1;
  for(unsigned i=0;i<r->count;i++)
    if(r->owners[i].bus==bus && r->owners[i].device==device &&
       r->owners[i].function==function && r->owners[i].pin==pin)
      return ppcvm_pci_irq_shared_set_level(&r->line,i,asserted);
  return -1;
}
