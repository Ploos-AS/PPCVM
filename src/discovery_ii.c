#include "ppcvm/discovery_ii.h"
#include <stdint.h>
#include <string.h>
uint32_t ppcvm_discovery_ii_swap32(uint32_t word) {
  return ((word & UINT32_C(0x000000ff)) << 24) |
         ((word & UINT32_C(0x0000ff00)) << 8) |
         ((word & UINT32_C(0x00ff0000)) >> 8) |
         ((word & UINT32_C(0xff000000)) >> 24);
}
static int config_register(uint32_t offset,unsigned *index,int *data) {
  if(offset==PPCVM_DISCOVERY_II_PCI0_CONFIG_ADDRESS_CANDIDATE) {*index=0;*data=0;return 1;}
  if(offset==PPCVM_DISCOVERY_II_PCI0_CONFIG_DATA_CANDIDATE) {*index=0;*data=1;return 1;}
  if(offset==PPCVM_DISCOVERY_II_PCI1_CONFIG_ADDRESS_CANDIDATE) {*index=1;*data=0;return 1;}
  if(offset==PPCVM_DISCOVERY_II_PCI1_CONFIG_DATA_CANDIDATE) {*index=1;*data=1;return 1;}
  return 0;
}
static ppcvm_bus_result read_register(void *context,uint32_t offset,uint32_t *value) {
  ppcvm_discovery_ii *c=(ppcvm_discovery_ii *)context;
  unsigned i=0;int data=0;
  if(!c || !value) return PPCVM_BUS_INVALID;
  if(c->pci_config_enabled && config_register(offset,&i,&data)) {
    uint32_t result=UINT32_MAX;
    if(!data) result=c->config_address[i];
    else if(c->pci[i] && (c->config_address[i]&UINT32_C(0x80000000))) {
      uint32_t a=c->config_address[i];
      (void)ppcvm_pci_bus_read32(c->pci[i],(uint8_t)(a>>16),
        (uint8_t)((a>>11)&31u),(uint8_t)((a>>8)&7u),a&252u,&result);
    }
    *value=ppcvm_discovery_ii_swap32(result);
    return PPCVM_BUS_OK;
  }
  c->unsupported_reads++;
  return PPCVM_BUS_UNMAPPED;
}
static ppcvm_bus_result write_register(void *context,uint32_t offset,uint32_t value) {
  ppcvm_discovery_ii *c=(ppcvm_discovery_ii *)context;
  unsigned i=0;int data=0;
  if(!c) return PPCVM_BUS_INVALID;
  if(c->pci_config_enabled && config_register(offset,&i,&data)) {
    uint32_t word=ppcvm_discovery_ii_swap32(value);
    if(!data) c->config_address[i]=word;
    else if(c->pci[i] && (c->config_address[i]&UINT32_C(0x80000000))) {
      uint32_t a=c->config_address[i];
      (void)ppcvm_pci_bus_write32(c->pci[i],(uint8_t)(a>>16),
        (uint8_t)((a>>11)&31u),(uint8_t)((a>>8)&7u),a&252u,word);
    }
    return PPCVM_BUS_OK;
  }
  c->unsupported_writes++;
  return PPCVM_BUS_UNMAPPED;
}
void ppcvm_discovery_ii_enable_pci_config(ppcvm_discovery_ii *c,
    ppcvm_pci_bus *pci0,ppcvm_pci_bus *pci1) {
  if(!c) return;
  c->pci[0]=pci0;c->pci[1]=pci1;
  c->config_address[0]=0;c->config_address[1]=0;
  c->pci_config_enabled=1;
}
void ppcvm_discovery_ii_init(ppcvm_discovery_ii *controller) {
  if(controller) memset(controller,0,sizeof(*controller));
}
void ppcvm_discovery_ii_reset(ppcvm_discovery_ii *controller) {
  if(!controller) return;
  controller->config_address[0]=0;
  controller->config_address[1]=0;
  controller->reset_count++;
  controller->unsupported_reads=0;
  controller->unsupported_writes=0;
}
ppcvm_bus_result ppcvm_discovery_ii_map(ppcvm_bus *bus,
    ppcvm_discovery_ii *controller,uint32_t base,uint32_t size) {
  if(!bus || !controller || !size || (base&3u) || (size&3u) ||
     (uint64_t)base+(uint64_t)size>UINT64_C(0x100000000))
    return PPCVM_BUS_INVALID;
  return ppcvm_bus_map_mmio32(bus,base,size,controller,read_register,write_register);
}
