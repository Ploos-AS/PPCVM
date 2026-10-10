#include "ppcvm/pci_irq_device.h"
#include <stddef.h>
int ppcvm_pci_irq_device_init(ppcvm_pci_irq_device *d,
    ppcvm_pci_irq_matrix *m,uint8_t bus,uint8_t device,
    uint8_t function,uint8_t pin) {
  if(!d || !m || pin<1 || pin>4) return -1;
  d->matrix=m;d->bus=bus;d->device=device;
  d->function=function;d->pin=pin;d->control=0;
  return 0;
}
int ppcvm_pci_irq_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value) {
  ppcvm_pci_irq_device *d=(ppcvm_pci_irq_device *)context;
  if(!d || !value || bar!=0 || (offset!=0 && offset!=4)) return -1;
  *value=d->control&1u;
  return 0;
}
int ppcvm_pci_irq_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value) {
  ppcvm_pci_irq_device *d=(ppcvm_pci_irq_device *)context;
  if(!d || !d->matrix || bar!=0 || offset!=0 || (value&~UINT32_C(1))) return -1;
  if(ppcvm_pci_irq_matrix_set_level(d->matrix,d->bus,d->device,d->function,d->pin,(int)(value&1u))!=0) return -1;
  d->control=value;
  return 0;
}
