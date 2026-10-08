#include "ppcvm/discovery_ii.h"
#include <stdint.h>
#include <string.h>
static ppcvm_bus_result read_register(void *context,uint32_t offset,uint32_t *value) {
  ppcvm_discovery_ii *controller=(ppcvm_discovery_ii *)context;
  (void)offset;
  if(!controller || !value) return PPCVM_BUS_INVALID;
  controller->unsupported_reads++;
  return PPCVM_BUS_UNMAPPED;
}
static ppcvm_bus_result write_register(void *context,uint32_t offset,uint32_t value) {
  ppcvm_discovery_ii *controller=(ppcvm_discovery_ii *)context;
  (void)offset;
  (void)value;
  if(!controller) return PPCVM_BUS_INVALID;
  controller->unsupported_writes++;
  return PPCVM_BUS_UNMAPPED;
}
void ppcvm_discovery_ii_init(ppcvm_discovery_ii *controller) {
  if(controller) memset(controller,0,sizeof(*controller));
}
void ppcvm_discovery_ii_reset(ppcvm_discovery_ii *controller) {
  if(!controller) return;
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
