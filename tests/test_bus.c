#include "ppcvm/bus.h"
#include <assert.h>
#include <stdint.h>
static ppcvm_bus_result read_reg(void *ctx, uint32_t offset, uint8_t *v) {
  uint8_t *p=(uint8_t *)ctx; *v=p[offset]; return PPCVM_BUS_OK;
}
static ppcvm_bus_result write_reg(void *ctx, uint32_t offset, uint8_t v) {
  uint8_t *p=(uint8_t *)ctx; p[offset]=v; return PPCVM_BUS_OK;
}
int main(void) {
  ppcvm_bus b;
  uint8_t ram[16]={0}, rom[4]={0x12,0x34,0x56,0x78}, regs[4]={0};
  uint8_t v=0; uint32_t w=0;
  ppcvm_bus_init(&b);
  assert(ppcvm_bus_map_memory(&b,0,16,ram,0)==PPCVM_BUS_OK);
  assert(ppcvm_bus_map_memory(&b,8,4,rom,1)==PPCVM_BUS_INVALID);
  assert(ppcvm_bus_map_memory(&b,0x1000,4,rom,1)==PPCVM_BUS_OK);
  assert(ppcvm_bus_map_mmio(&b,0x2000,4,regs,read_reg,write_reg)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&b,4,0xdeadbeefu)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&b,4,&w)==PPCVM_BUS_OK && w==0xdeadbeefu);
  assert(ppcvm_bus_read32be(&b,0x1000,&w)==PPCVM_BUS_OK && w==0x12345678u);
  assert(ppcvm_bus_write8(&b,0x1000,1)==PPCVM_BUS_READ_ONLY);
  assert(ppcvm_bus_write32be(&b,0x1000,1)==PPCVM_BUS_READ_ONLY);
  assert(ppcvm_bus_write8(&b,0x2001,0xa5)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read8(&b,0x2001,&v)==PPCVM_BUS_OK && v==0xa5);
  assert(ppcvm_bus_read32be(&b,0x2000,&w)==PPCVM_BUS_INVALID);
  assert(ppcvm_bus_read8(&b,0x3000,&v)==PPCVM_BUS_UNMAPPED);
  assert(ppcvm_bus_read32be(&b,14,&w)==PPCVM_BUS_UNMAPPED);
  assert(ppcvm_bus_map_memory(&b,0xfffffffeu,4,rom,1)==PPCVM_BUS_INVALID);
  return 0;
}
