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
  if(!d || index>=6u || d->bar64[index] || (index>0u && d->bar64[index-1u]) || size<16u || (size&(size-1u)) ||
     (base&(size-1u))) return -1;
  d->bar_size[index]=size;
  d->bar_probe[index]=0;
  d->bar_io[index]=0;
  uint32_t value=base&UINT32_C(0xfffffff0);
  uint8_t *p=d->config+0x10u+index*4u;
  for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(value>>(8u*i));
  return 0;
}
int ppcvm_pci_set_io_bar32(ppcvm_pci_device *d,unsigned index,
                            uint32_t size,uint32_t base) {
  if(!d || index>=6u || d->bar64[index] || (index>0u && d->bar64[index-1u]) || size<4u || (size&(size-1u)) ||
     (base&(size-1u))) return -1;
  d->bar_size[index]=size;
  d->bar_probe[index]=0;
  d->bar_io[index]=1;
  uint32_t value=(base&UINT32_C(0xfffffffc))|1u;
  uint8_t *p=d->config+0x10u+index*4u;
  for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(value>>(8u*i));
  return 0;
}
int ppcvm_pci_set_mem_bar64(ppcvm_pci_device *d,unsigned index,
                             uint64_t size,uint64_t base) {
  if(!d || index>=5u || size<16u || (size&(size-1u)) ||
     (base&(size-1u)) || d->bar_size[index] || d->bar_size[index+1u] ||
     d->bar64[index] || (index>0u && d->bar64[index-1u])) return -1;
  d->bar64[index]=1;
  d->bar64_size[index]=size;
  d->bar_probe[index]=0;
  d->bar_probe[index+1u]=0;
  uint32_t lo=((uint32_t)base&UINT32_C(0xfffffff0))|4u;
  uint32_t hi=(uint32_t)(base>>32);
  for(unsigned i=0;i<4;i++) {
    d->config[0x10u+index*4u+i]=(uint8_t)(lo>>(8u*i));
    d->config[0x14u+index*4u+i]=(uint8_t)(hi>>(8u*i));
  }
  return 0;
}
int ppcvm_pci_read32(const ppcvm_pci_device *d,uint32_t offset,uint32_t *value) {
  if (!d || !value || (offset&3u) || offset>252u) return -1;
  if(offset>=0x10u && offset<=0x24u) {
    unsigned index=(offset-0x10u)/4u;
    if(d->bar64[index] && d->bar_probe[index]) {
      uint64_t mask=~(d->bar64_size[index]-UINT64_C(1));
      *value=((uint32_t)mask&UINT32_C(0xfffffff0))|4u;
      return 0;
    }
    if(index>0u && d->bar64[index-1u] && d->bar_probe[index]) {
      uint64_t mask=~(d->bar64_size[index-1u]-UINT64_C(1));
      *value=(uint32_t)(mask>>32);
      return 0;
    }
    if(d->bar_size[index] && d->bar_probe[index]) {
      *value=(~(d->bar_size[index]-1u) &
        (d->bar_io[index] ? UINT32_C(0xfffffffc) : UINT32_C(0xfffffff0))) |
        (d->bar_io[index] ? 1u : 0u);
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
  /* Command register bits 0-2 are writable; status is read-only here. */
  if(offset==4u) {
    d->config[4]=(uint8_t)(value&7u);
    d->config[5]=0;
    return 0;
  }
  if(offset>=0x10u && offset<=0x24u) {
    unsigned index=(offset-0x10u)/4u;
    if(d->bar64[index] || (index>0u && d->bar64[index-1u])) {
      unsigned owner=d->bar64[index] ? index : index-1u;
      if(value==UINT32_MAX) {
        d->bar_probe[index]=1;
        return 0;
      }
      d->bar_probe[index]=0;
      uint32_t mask=index==owner ?
        (uint32_t)(~(d->bar64_size[owner]-UINT64_C(1)))&UINT32_C(0xfffffff0) :
        (uint32_t)((~(d->bar64_size[owner]-UINT64_C(1)))>>32);
      value &= mask;
      if(index==owner) value |= 4u;
    } else if(d->bar_size[index]) {
      if(value==UINT32_MAX) {
        d->bar_probe[index]=1;
        return 0;
      }
      d->bar_probe[index]=0;
      value &= ~(d->bar_size[index]-1u);
      value &= d->bar_io[index] ? UINT32_C(0xfffffffc) : UINT32_C(0xfffffff0);
      if(d->bar_io[index]) value |= 1u;
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

static uint32_t cfg32(const ppcvm_pci_device *d,unsigned index) {
  const uint8_t *p=d->config+0x10u+index*4u;
  return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
int ppcvm_pci_bus_decode_memory(const ppcvm_pci_bus *b,uint64_t address,
                                 ppcvm_pci_bar_hit *hit) {
  if(!b || !hit) return -1;
  int found=0;
  ppcvm_pci_bar_hit candidate={0};
  for(size_t n=0;n<b->count;n++) {
    const ppcvm_pci_slot *slot=&b->slots[n];
    const ppcvm_pci_device *d=&slot->config;
    if(!(d->config[4]&2u)) continue; /* PCI Command: Memory Space Enable */
    for(unsigned i=0;i<6;i++) {
      uint64_t size=0,base=0;
      if(d->bar64[i]) {
        size=d->bar64_size[i];
        base=((uint64_t)cfg32(d,i+1u)<<32)|
             ((uint64_t)cfg32(d,i)&UINT64_C(0xfffffff0));
      } else if(i>0u && d->bar64[i-1u]) {
        continue;
      } else if(d->bar_size[i] && !d->bar_io[i]) {
        size=d->bar_size[i];
        base=(uint64_t)(cfg32(d,i)&UINT32_C(0xfffffff0));
      }
      if(!size || address<base || address-base>=size) continue;
      if(found) return -1;
      found=1;
      candidate.bus=slot->bus;
      candidate.device=slot->device;
      candidate.function=slot->function;
      candidate.bar_index=(uint8_t)i;
      candidate.offset=address-base;
    }
  }
  if(found) *hit=candidate;
  return found ? 0 : 1;
}

int ppcvm_pci_bus_decode_io(const ppcvm_pci_bus *b,uint32_t address,
                             ppcvm_pci_bar_hit *hit) {
  if(!b || !hit) return -1;
  int found=0;
  ppcvm_pci_bar_hit candidate={0};
  for(size_t n=0;n<b->count;n++) {
    const ppcvm_pci_slot *slot=&b->slots[n];
    const ppcvm_pci_device *d=&slot->config;
    if(!(d->config[4]&1u)) continue; /* PCI Command: I/O Space Enable */
    for(unsigned i=0;i<6;i++) {
      if(!d->bar_size[i] || !d->bar_io[i]) continue;
      uint32_t base=cfg32(d,i)&UINT32_C(0xfffffffc);
      if(address<base || (uint64_t)address-base>=d->bar_size[i]) continue;
      if(found) return -1;
      found=1;
      candidate.bus=slot->bus;
      candidate.device=slot->device;
      candidate.function=slot->function;
      candidate.bar_index=(uint8_t)i;
      candidate.offset=(uint64_t)address-base;
    }
  }
  if(found) *hit=candidate;
  return found ? 0 : 1;
}
