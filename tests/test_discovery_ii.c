#include "ppcvm/discovery_ii.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bus bus;
  ppcvm_discovery_ii controller;
  uint32_t value=0x12345678u;
  ppcvm_bus_init(&bus);
  ppcvm_discovery_ii_init(&controller);
  assert(controller.reset_count==0 && controller.unsupported_reads==0);
  assert(ppcvm_discovery_ii_map(&bus,&controller,0x80000001u,4096)==PPCVM_BUS_INVALID);
  assert(ppcvm_discovery_ii_map(&bus,&controller,0x80000000u,4095)==PPCVM_BUS_INVALID);
  assert(ppcvm_discovery_ii_map(&bus,&controller,0xfffff000u,8192)==PPCVM_BUS_INVALID);
  assert(ppcvm_discovery_ii_map(&bus,&controller,0x80000000u,4096)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000000u,&value)==PPCVM_BUS_UNMAPPED);
  assert(value==0x12345678u);
  assert(controller.unsupported_reads==1);
  assert(ppcvm_bus_write32be(&bus,0x80000004u,0xfeedbeefu)==PPCVM_BUS_UNMAPPED);
  assert(controller.unsupported_writes==1);
  ppcvm_discovery_ii_reset(&controller);
  assert(controller.reset_count==1);
  assert(controller.unsupported_reads==0 && controller.unsupported_writes==0);
  assert(ppcvm_bus_read32be(&bus,0x80000000u,&value)==PPCVM_BUS_UNMAPPED);
  assert(controller.unsupported_reads==1);
  return 0;
}
