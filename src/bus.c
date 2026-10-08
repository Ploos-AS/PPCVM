#include "ppcvm/bus.h"
#include <string.h>
static int overlap(uint32_t a, uint32_t as, uint32_t b, uint32_t bs) {
  return (uint64_t)a < (uint64_t)b + bs && (uint64_t)b < (uint64_t)a + as;
}
static ppcvm_bus_result add(ppcvm_bus *bus, ppcvm_bus_region region) {
  if (!bus || !region.size || (uint64_t)region.base + region.size > UINT64_C(0x100000000) || bus->count >= PPCVM_MAX_REGIONS)
    return PPCVM_BUS_INVALID;
  for (size_t i=0; i<bus->count; ++i)
    if (overlap(region.base, region.size, bus->regions[i].base, bus->regions[i].size))
      return PPCVM_BUS_INVALID;
  bus->regions[bus->count++] = region;
  return PPCVM_BUS_OK;
}
void ppcvm_bus_init(ppcvm_bus *bus) { if (bus) memset(bus, 0, sizeof(*bus)); }
ppcvm_bus_result ppcvm_bus_map_memory(ppcvm_bus *bus, uint32_t base, uint32_t size, uint8_t *bytes, int read_only) {
  if (!bytes) return PPCVM_BUS_INVALID;
  ppcvm_bus_region region = {0};
  region.base=base; region.size=size; region.bytes=bytes;
  region.kind=read_only ? PPCVM_REGION_ROM : PPCVM_REGION_RAM;
  return add(bus, region);
}
ppcvm_bus_result ppcvm_bus_map_mmio(ppcvm_bus *bus, uint32_t base, uint32_t size, void *context, ppcvm_mmio_read read, ppcvm_mmio_write write) {
  if (!read || !write) return PPCVM_BUS_INVALID;
  ppcvm_bus_region region = {0};
  region.base=base; region.size=size; region.kind=PPCVM_REGION_MMIO;
  region.context=context; region.read=read; region.write=write;
  return add(bus, region);
}
static ppcvm_bus_region *find(ppcvm_bus *bus, uint32_t address, uint32_t width) {
  if (!bus) return NULL;
  for (size_t i=0; i<bus->count; ++i) {
    ppcvm_bus_region *r=&bus->regions[i];
    if ((uint64_t)address >= r->base && (uint64_t)address + width <= (uint64_t)r->base + r->size)
      return r;
  }
  return NULL;
}
ppcvm_bus_result ppcvm_bus_read8(ppcvm_bus *bus, uint32_t address, uint8_t *value) {
  if (!value) return PPCVM_BUS_INVALID;
  ppcvm_bus_region *r=find(bus,address,1);
  if (!r) return PPCVM_BUS_UNMAPPED;
  uint32_t offset=address-r->base;
  if (r->kind==PPCVM_REGION_MMIO) return r->read(r->context,offset,value);
  *value=r->bytes[offset];
  return PPCVM_BUS_OK;
}
ppcvm_bus_result ppcvm_bus_write8(ppcvm_bus *bus, uint32_t address, uint8_t value) {
  ppcvm_bus_region *r=find(bus,address,1);
  if (!r) return PPCVM_BUS_UNMAPPED;
  if (r->kind==PPCVM_REGION_ROM) return PPCVM_BUS_READ_ONLY;
  uint32_t offset=address-r->base;
  if (r->kind==PPCVM_REGION_MMIO) return r->write(r->context,offset,value);
  r->bytes[offset]=value;
  return PPCVM_BUS_OK;
}
ppcvm_bus_result ppcvm_bus_read32be(ppcvm_bus *bus, uint32_t address, uint32_t *value) {
  if (!value || (address & 3u)) return PPCVM_BUS_INVALID;
  ppcvm_bus_region *r=find(bus,address,4);
  if (!r) return PPCVM_BUS_UNMAPPED;
  if (r->kind==PPCVM_REGION_MMIO) return PPCVM_BUS_INVALID; /* register-width MMIO pending */
  uint32_t offset=address-r->base;
  const uint8_t *p=r->bytes+offset;
  *value=((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
  return PPCVM_BUS_OK;
}
ppcvm_bus_result ppcvm_bus_write32be(ppcvm_bus *bus, uint32_t address, uint32_t value) {
  if (address & 3u) return PPCVM_BUS_INVALID;
  ppcvm_bus_region *r=find(bus,address,4);
  if (!r) return PPCVM_BUS_UNMAPPED;
  if (r->kind==PPCVM_REGION_ROM) return PPCVM_BUS_READ_ONLY;
  if (r->kind==PPCVM_REGION_MMIO) return PPCVM_BUS_INVALID;
  uint32_t offset=address-r->base;
  uint8_t *p=r->bytes+offset;
  p[0]=(uint8_t)(value>>24); p[1]=(uint8_t)(value>>16); p[2]=(uint8_t)(value>>8); p[3]=(uint8_t)value;
  return PPCVM_BUS_OK;
}
