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
int ppcvm_pci_event_device_reset(ppcvm_pci_event_device *d) {
  if(!d || !d->matrix) return -1;
  return apply(d,0u,0u);
}
int ppcvm_pci_event_device_save(const ppcvm_pci_event_device *d,
    ppcvm_pci_event_snapshot *snapshot) {
  if(!d || !d->matrix || !snapshot) return -1;
  snapshot->version=PPCVM_PCI_EVENT_SNAPSHOT_VERSION;
  snapshot->pending=d->pending;
  snapshot->enable=d->enable;
  return 0;
}
int ppcvm_pci_event_device_restore(ppcvm_pci_event_device *d,
    const ppcvm_pci_event_snapshot *snapshot) {
  if(!d || !d->matrix || !snapshot ||
      snapshot->version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((snapshot->pending|snapshot->enable)&~EVENT_BITS)!=0u) return -1;
  return apply(d,snapshot->pending,snapshot->enable);
}
static void put_be32(uint8_t *p,uint32_t v) {
  p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);
  p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;
}
static uint32_t get_be32(const uint8_t *p) {
  return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
      ((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
int ppcvm_pci_event_snapshot_encode(const ppcvm_pci_event_snapshot *snapshot,
    uint8_t *out,size_t size) {
  if(!snapshot || !out || size<PPCVM_PCI_EVENT_WIRE_SIZE ||
      snapshot->version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((snapshot->pending|snapshot->enable)&~EVENT_BITS)!=0u) return -1;
  out[0]='P';out[1]='E';out[2]='V';out[3]='T';
  put_be32(out+4,snapshot->version);
  put_be32(out+8,snapshot->pending);
  put_be32(out+12,snapshot->enable);
  return 0;
}
int ppcvm_pci_event_snapshot_decode(ppcvm_pci_event_snapshot *snapshot,
    const uint8_t *data,size_t size) {
  ppcvm_pci_event_snapshot temp;
  if(!snapshot || !data || size!=PPCVM_PCI_EVENT_WIRE_SIZE ||
      data[0]!='P' || data[1]!='E' || data[2]!='V' || data[3]!='T') return -1;
  temp.version=get_be32(data+4);
  temp.pending=get_be32(data+8);
  temp.enable=get_be32(data+12);
  if(temp.version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((temp.pending|temp.enable)&~EVENT_BITS)!=0u) return -1;
  *snapshot=temp;
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
