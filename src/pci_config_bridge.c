#include "ppcvm/pci_config_bridge.h"
#include <stdint.h>

static ppcvm_bus_result read_register(void *ctx,uint32_t offset,uint32_t *value) {
  ppcvm_pci_config_bridge *bridge=(ppcvm_pci_config_bridge *)ctx;
  if(!bridge || !value) return PPCVM_BUS_INVALID;
  if(offset==0) {*value=bridge->address;return PPCVM_BUS_OK;}
  if(offset!=4) return PPCVM_BUS_UNMAPPED;
  if(!(bridge->address&UINT32_C(0x80000000))) {*value=UINT32_MAX;return PPCVM_BUS_OK;}
  uint32_t a=bridge->address;
  int result=ppcvm_pci_bus_read32(bridge->pci,(uint8_t)(a>>16),
    (uint8_t)((a>>11)&31u),(uint8_t)((a>>8)&7u),a&252u,value);
  if(result!=0) *value=UINT32_MAX;
  return PPCVM_BUS_OK;
}
static ppcvm_bus_result write_register(void *ctx,uint32_t offset,uint32_t value) {
  ppcvm_pci_config_bridge *bridge=(ppcvm_pci_config_bridge *)ctx;
  if(!bridge) return PPCVM_BUS_INVALID;
  if(offset==0) {bridge->address=value;return PPCVM_BUS_OK;}
  if(offset!=4) return PPCVM_BUS_UNMAPPED;
  if(!(bridge->address&UINT32_C(0x80000000))) return PPCVM_BUS_OK;
  uint32_t a=bridge->address;
  (void)ppcvm_pci_bus_write32(bridge->pci,(uint8_t)(a>>16),
    (uint8_t)((a>>11)&31u),(uint8_t)((a>>8)&7u),a&252u,value);
  return PPCVM_BUS_OK;
}
ppcvm_bus_result ppcvm_pci_map_config_bridge(ppcvm_bus *cpu_bus,
    ppcvm_pci_config_bridge *bridge,ppcvm_pci_bus *pci,uint32_t base) {
  if(!cpu_bus || !bridge || !pci || (base&3u) ||
     (uint64_t)base+8u>UINT64_C(0x100000000))
    return PPCVM_BUS_INVALID;
  bridge->pci=pci;
  bridge->address=0;
  return ppcvm_bus_map_mmio32(cpu_bus,base,8,bridge,read_register,write_register);
}
