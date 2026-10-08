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
int ppcvm_pci_read32(const ppcvm_pci_device *d,uint32_t offset,uint32_t *value) {
  if (!d || !value || (offset&3u) || offset>252u) return -1;
  const uint8_t *p=d->config+offset;
  *value=(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
  return 0;
}
int ppcvm_pci_write32(ppcvm_pci_device *d,uint32_t offset,uint32_t value) {
  if (!d || (offset&3u) || offset>252u) return -1;
  /* Identification and class/revision registers are read-only. */
  if (offset==0u || offset==8u) return 0;
  uint8_t *p=d->config+offset;
  for (unsigned i=0;i<4;i++) p[i]=(uint8_t)(value>>(i*8u));
  return 0;
}
