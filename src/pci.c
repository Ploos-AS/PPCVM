#include "ppcvm/pci.h"
#include <string.h>
static void put16(uint8_t *p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
void ppcvm_pci_device_init(ppcvm_pci_device *d,uint16_t vendor,uint16_t product,
                           uint8_t class_code,uint8_t subclass,uint8_t revision) {
  if (!d) return;
  memset(d,0,sizeof(*d));
  put16(d->config,vendor);put16(d->config+2,product);
  d->config[8]=revision;d->config[10]=subclass;d->config[11]=class_code;
}
int ppcvm_pci_set_mem_bar32(ppcvm_pci_device *d,unsigned index,
                             uint32_t size,uint32_t base) {
  if(!d || index>=6u || size<16u || (size&(size-1u)) ||
     (base&(size-1u))) return -1;
  d->bar_size[index]=size;
  d->bar_probe[index]=0;
  uint32_t value=base&UINT32_C(0xfffffff0);
  uint8_t *p=d->config+0x10u+index*4u;
  for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(value>>(8u*i));
  return 0;
}
int ppcvm_pci_read32(const ppcvm_pci_device *d,uint32_t offset,uint32_t *value) {
  if (!d || !value || (offset&3u) || offset>252u) return -1;
  if(offset>=0x10u && offset<=0x24u) {
    unsigned index=(offset-0x10u)/4u;
    if(d->bar_size[index] && d->bar_probe[index]) {
      *value=~(d->bar_size[index]-1u)&UINT32_C(0xfffffff0);
      return 0;
    }
  }
  const uint8_t *p=d->config+offset;
  *value=(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
  return 0;
}
int ppcvm_pci_write32(ppcvm_pci_device *d,uint32_t offset,uint32_t value) {
  if (!d || (offset&3u) || offset>252u) return -1;
  /* Identification and class/revision registers are read-only. */
  if (offset==0u || offset==8u) return 0;
  if(offset>=0x10u && offset<=0x24u) {
    unsigned index=(offset-0x10u)/4u;
    if(d->bar_size[index]) {
      if(value==UINT32_MAX) {
        d->bar_probe[index]=1;
        return 0;
      }
      d->bar_probe[index]=0;
      value &= ~(d->bar_size[index]-1u);
      value &= UINT32_C(0xfffffff0);
    }
  }
  uint8_t *p=d->config+offset;
  for (unsigned i=0;i<4;i++) p[i]=(uint8_t)(value>>(i*8u));
  return 0;
}

static ppcvm_pci_slot *find_slot(ppcvm_pci_bus *b,uint8_t bus,uint8_t dev,uint8_t fn) {
  for(size_t i=0;i<b->count;i++)
    if(b->slots[i].bus==bus && b->slots[i].device==dev &&
       b->slots[i].function==fn) return &b->slots[i];
  return 0;
}
void ppcvm_pci_bus_init(ppcvm_pci_bus *b) {
  if(b) memset(b,0,sizeof(*b));
}
int ppcvm_pci_bus_add(ppcvm_pci_bus *b,uint8_t bus,uint8_t dev,
                      uint8_t fn,const ppcvm_pci_device *cfg) {
  if(!b || !cfg || dev>=32u || fn>=8u || b->count>=PPCVM_PCI_MAX_DEVICES ||
     find_slot(b,bus,dev,fn)) return -1;
  ppcvm_pci_slot *slot=&b->slots[b->count++];
  slot->bus=bus;slot->device=dev;slot->function=fn;slot->config=*cfg;
  return 0;
}
int ppcvm_pci_bus_read32(const ppcvm_pci_bus *b,uint8_t bus,uint8_t dev,
                         uint8_t fn,uint32_t offset,uint32_t *value) {
  if(!b || !value || dev>=32u || fn>=8u || (offset&3u) || offset>252u) return -1;
  for(size_t i=0;i<b->count;i++) {
    const ppcvm_pci_slot *s=&b->slots[i];
    if(s->bus==bus && s->device==dev && s->function==fn)
      return ppcvm_pci_read32(&s->config,offset,value);
  }
  *value=UINT32_MAX;
  return 0;
}
int ppcvm_pci_bus_write32(ppcvm_pci_bus *b,uint8_t bus,uint8_t dev,
                          uint8_t fn,uint32_t offset,uint32_t value) {
  if(!b || dev>=32u || fn>=8u || (offset&3u) || offset>252u) return -1;
  ppcvm_pci_slot *s=find_slot(b,bus,dev,fn);
  return s ? ppcvm_pci_write32(&s->config,offset,value) : 0;
}
