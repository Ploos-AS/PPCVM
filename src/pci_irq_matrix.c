#include "ppcvm/pci_irq_matrix.h"
#include <stddef.h>
int ppcvm_pci_irq_matrix_init(ppcvm_pci_irq_matrix *m,
    ppcvm_pci_bus *pci,ppcvm_discovery_ii *c) {
  if(!m || !pci || !c) return -1;
  m->pci=pci;m->controller=c;
  for(unsigned i=0;i<PPCVM_PCI_IRQ_MATRIX_SOURCES;i++)
    if(ppcvm_pci_irq_router_init(&m->sources[i],pci,c,(uint8_t)i)!=0) return -1;
  return 0;
}
int ppcvm_pci_irq_matrix_add(ppcvm_pci_irq_matrix *m,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,uint8_t source) {
  if(!m || !m->pci || source>=PPCVM_PCI_IRQ_MATRIX_SOURCES) return -1;
  /* BDF+pin must have exactly one destination across the entire matrix. */
  for(unsigned i=0;i<PPCVM_PCI_IRQ_MATRIX_SOURCES;i++)
    for(unsigned j=0;j<m->sources[i].count;j++) {
      const ppcvm_pci_irq_router *r=&m->sources[i];
      if(r->owners[j].bus==bus && r->owners[j].device==device &&
         r->owners[j].function==function && r->owners[j].pin==pin) return -1;
    }
  return ppcvm_pci_irq_router_add(&m->sources[source],bus,device,function,pin);
}
int ppcvm_pci_irq_matrix_set_level(ppcvm_pci_irq_matrix *m,
    uint8_t bus,uint8_t device,uint8_t function,uint8_t pin,int asserted) {
  if(!m || !m->pci) return -1;
  for(unsigned i=0;i<PPCVM_PCI_IRQ_MATRIX_SOURCES;i++)
    for(unsigned j=0;j<m->sources[i].count;j++) {
      const ppcvm_pci_irq_router *r=&m->sources[i];
      if(r->owners[j].bus==bus && r->owners[j].device==device &&
         r->owners[j].function==function && r->owners[j].pin==pin)
        return ppcvm_pci_irq_router_set_level(&m->sources[i],bus,device,function,pin,asserted);
    }
  return -1;
}
