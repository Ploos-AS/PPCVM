#include "ppcvm/pci_event_device.h"
#include <stddef.h>
#define EVENT_BITS UINT32_C(0x0f)
static int apply(ppcvm_pci_event_device *d,uint32_t pending,uint32_t enable) {
  if(ppcvm_pci_irq_matrix_set_level(d->matrix,d->bus,d->device,
      d->function,d->pin,(pending&enable)!=0u)!=0) return -1;
  d->pending=pending;d->enable=enable;
  return 0;
}
int ppcvm_pci_event_device_init(ppcvm_pci_event_device *d,
    ppcvm_pci_irq_matrix *matrix,uint8_t bus,uint8_t device,
    uint8_t function,uint8_t pin) {
  if(!d || !matrix || pin<1 || pin>4) return -1;
  d->matrix=matrix;d->bus=bus;d->device=device;d->function=function;
  d->pin=pin;d->pending=0;d->enable=0;
  return 0;
}
int ppcvm_pci_event_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value) {
  ppcvm_pci_event_device *d=(ppcvm_pci_event_device *)context;
  if(!d || !value || bar!=0) return -1;
  switch(offset) {
    case 0: *value=d->pending;return 0;
    case 4: *value=d->enable;return 0;
    case 8: case 12: *value=0;return 0;
    default:return -1;
  }
}
int ppcvm_pci_event_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value) {
  ppcvm_pci_event_device *d=(ppcvm_pci_event_device *)context;
  if(!d || !d->matrix || bar!=0 || (value&~EVENT_BITS)!=0) return -1;
  switch(offset) {
    case 4:return apply(d,d->pending,value);
    case 8:return apply(d,d->pending&~value,d->enable);
    case 12:return apply(d,d->pending|value,d->enable);
    default:return -1;
  }
}
