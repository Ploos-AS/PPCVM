#include "ppcvm/pci_config_bridge.h"
#include <assert.h>
#include <stdint.h>

int main(void) {
  ppcvm_bus cpu_bus;
  ppcvm_pci_bus pci;
  ppcvm_pci_device device;
  ppcvm_pci_config_bridge bridge;
  uint32_t value=0;
  const uint32_t base=0x80000000u;
  const uint32_t bdf=0x80001000u; /* bus 0, device 2, function 0 */
  ppcvm_bus_init(&cpu_bus);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,0x02,0,1);
  assert(ppcvm_pci_set_mem_bar32(&device,0,4096,0x90000000u)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  assert(ppcvm_pci_map_config_bridge(&cpu_bus,&bridge,&pci,base)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX); /* disabled configuration cycles */
  assert(ppcvm_bus_write32be(&cpu_bus,base,bdf)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base,&value)==PPCVM_BUS_OK);
  assert(value==bdf);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==0x56781234u);
  assert(ppcvm_bus_write32be(&cpu_bus,base,bdf+8)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==0x02000001u);
  assert(ppcvm_bus_write32be(&cpu_bus,base,bdf+0x10)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==0x90000000u);
  assert(ppcvm_bus_write32be(&cpu_bus,base+4,UINT32_MAX)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==0xfffff000u);
  assert(ppcvm_bus_write32be(&cpu_bus,base+4,0x90000000u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&cpu_bus,base,bdf+4)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&cpu_bus,base+4,2)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert((value&2u)!=0);
  assert(ppcvm_bus_write32be(&cpu_bus,base,0x80001800u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX); /* missing BDF */
  assert(ppcvm_bus_write32be(&cpu_bus,base,0x00001000u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu_bus,base+4,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(ppcvm_pci_map_config_bridge(&cpu_bus,&bridge,&pci,base+2)==PPCVM_BUS_INVALID);
  return 0;
}
