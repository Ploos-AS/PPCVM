#ifndef PPCVM_BUS_H
#define PPCVM_BUS_H
#include <stddef.h>
#include <stdint.h>
typedef enum { PPCVM_BUS_OK=0, PPCVM_BUS_UNMAPPED=1, PPCVM_BUS_READ_ONLY=2, PPCVM_BUS_INVALID=3 } ppcvm_bus_result;
typedef ppcvm_bus_result (*ppcvm_mmio_read)(void *context, uint32_t offset, uint8_t *value);
typedef ppcvm_bus_result (*ppcvm_mmio_write)(void *context, uint32_t offset, uint8_t value);
typedef ppcvm_bus_result (*ppcvm_mmio_read32)(void *context, uint32_t offset, uint32_t *value);
typedef ppcvm_bus_result (*ppcvm_mmio_write32)(void *context, uint32_t offset, uint32_t value);
typedef enum { PPCVM_REGION_RAM, PPCVM_REGION_ROM, PPCVM_REGION_MMIO } ppcvm_region_kind;
typedef struct {
  uint32_t base;
  uint32_t size;
  ppcvm_region_kind kind;
  uint8_t *bytes;
  void *context;
  ppcvm_mmio_read read;
  ppcvm_mmio_write write;
  ppcvm_mmio_read32 read32;
  ppcvm_mmio_write32 write32;
} ppcvm_bus_region;
#define PPCVM_MAX_REGIONS 32
typedef struct { ppcvm_bus_region regions[PPCVM_MAX_REGIONS]; size_t count; } ppcvm_bus;
void ppcvm_bus_init(ppcvm_bus *bus);
ppcvm_bus_result ppcvm_bus_map_memory(ppcvm_bus *bus, uint32_t base, uint32_t size, uint8_t *bytes, int read_only);
ppcvm_bus_result ppcvm_bus_map_mmio(ppcvm_bus *bus, uint32_t base, uint32_t size, void *context, ppcvm_mmio_read read, ppcvm_mmio_write write);
ppcvm_bus_result ppcvm_bus_map_mmio32(ppcvm_bus *bus, uint32_t base, uint32_t size, void *context, ppcvm_mmio_read32 read, ppcvm_mmio_write32 write);
ppcvm_bus_result ppcvm_bus_read8(ppcvm_bus *bus, uint32_t address, uint8_t *value);
ppcvm_bus_result ppcvm_bus_write8(ppcvm_bus *bus, uint32_t address, uint8_t value);
ppcvm_bus_result ppcvm_bus_read32be(ppcvm_bus *bus, uint32_t address, uint32_t *value);
ppcvm_bus_result ppcvm_bus_write32be(ppcvm_bus *bus, uint32_t address, uint32_t value);
#endif
