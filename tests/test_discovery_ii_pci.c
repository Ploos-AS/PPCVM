#include "ppcvm/discovery_ii.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_bus bus;ppcvm_discovery_ii c;
  ppcvm_pci_bus p0,p1;
  ppcvm_pci_device d0,d1;
  uint32_t value=0;
  ppcvm_bus_init(&bus);
  ppcvm_discovery_ii_init(&c);
  ppcvm_pci_bus_init(&p0);ppcvm_pci_bus_init(&p1);
  ppcvm_pci_device_init(&d0,0x1234,0x5678,0,0,0);
  ppcvm_pci_device_init(&d1,0xabcd,0xef01,0,0,0);
  assert(ppcvm_pci_bus_add(&p0,0,2,0,&d0)==0);
  assert(ppcvm_pci_bus_add(&p1,0,2,0,&d1)==0);
  assert(ppcvm_discovery_ii_map(&bus,&c,0x80000000u,4096)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cf8u,&value)==PPCVM_BUS_UNMAPPED);
  ppcvm_discovery_ii_enable_pci_config(&c,&p0,&p1);
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cf8u,&value)==PPCVM_BUS_OK);
  assert(value==0x00100080u);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0x34127856u);
  assert(ppcvm_bus_write32be(&bus,0x80000c78u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000c7cu,&value)==PPCVM_BUS_OK);
  assert(value==0xcdab01efu);
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0xffffffffu);
  /* A non-existent slot must not alias a populated slot. */
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0x00180080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  /* Configuration address latches are independent across PCI interfaces. */
  assert(ppcvm_bus_read32be(&bus,0x80000c78u,&value)==PPCVM_BUS_OK);
  assert(value==0x00100080u);
  /* Disabled cycle must not modify a device configuration register. */
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0x00100000u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,0x80000cfcu,0x00000000u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0x34127856u);
  /* Reset must clear both address latches without detaching the PCI buses. */
  assert(ppcvm_bus_write32be(&bus,0x80000c78u,0x00100080u)==PPCVM_BUS_OK);
  ppcvm_discovery_ii_reset(&c);
  assert(c.config_address[0]==0 && c.config_address[1]==0);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(c.pci_config_enabled==1u && c.pci[0]==&p0 && c.pci[1]==&p1);
  assert(ppcvm_bus_read32be(&bus,0x80000c7cu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(ppcvm_bus_write32be(&bus,0x80000cf8u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0x34127856u);
  assert(ppcvm_bus_write32be(&bus,0x80000c78u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0x80000c7cu,&value)==PPCVM_BUS_OK);
  assert(value==0xcdab01efu);
  return 0;
}
