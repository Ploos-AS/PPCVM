#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_device d;
  ppcvm_pci_bus bus;
  uint32_t v=0;
  ppcvm_pci_device_init(&d,0x1234,0x5678,2,0,0);
  assert(ppcvm_pci_set_mem_bar32(&d,0,4096,0x80000000)==0);
  assert(ppcvm_pci_set_mem_bar32(&d,1,256,0x80001000)==0);
  assert(ppcvm_pci_set_mem_bar32(&d,6,4096,0)!=0);
  assert(ppcvm_pci_set_mem_bar32(&d,2,300,0)!=0);
  assert(ppcvm_pci_set_mem_bar32(&d,2,4096,0x1234)!=0);
  ppcvm_pci_bus_init(&bus);
  assert(ppcvm_pci_bus_add(&bus,0,2,0,&d)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x10,&v)==0 && v==0x80000000);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x10,UINT32_MAX)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x10,&v)==0 && v==0xfffff000);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x14,&v)==0 && v==0x80001000);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x10,0x90001234)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x10,&v)==0 && v==0x90001000);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x14,UINT32_MAX)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x14,&v)==0 && v==0xffffff00);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x14,0x90002000)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x14,&v)==0 && v==0x90002000);
  return 0;
}
