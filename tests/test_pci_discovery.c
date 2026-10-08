#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>

/* Firmware-side PCI enumeration prototype: BDF probing, BAR sizing,
 * command register decode gating, and missing-device behavior. */
int main(void) {
  ppcvm_pci_bus bus;
  ppcvm_pci_device dev;
  uint32_t value=0;
  ppcvm_pci_bar_hit hit;
  ppcvm_pci_bus_init(&bus);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,0x02,0x00,0x01);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000u)==0);
  assert(ppcvm_pci_bus_add(&bus,0,2,0,&dev)==0);
  unsigned found=0;
  for(unsigned device=0;device<32;device++) {
    if(ppcvm_pci_bus_read32(&bus,0,(uint8_t)device,0,0,&value)!=0)
      continue;
    if((value&0xffffu)==0xffffu) continue;
    assert(device==2);
    assert(value==0x56781234u);
    found++;
  }
  assert(found==1);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,8,&value)==0);
  assert(value==0x02000001u);
  assert(ppcvm_pci_bus_decode_memory(&bus,0x90000004u,&hit)==1);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x10,&value)==0);
  assert(value==0x90000000u);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x10,UINT32_MAX)==0);
  assert(ppcvm_pci_bus_read32(&bus,0,2,0,0x10,&value)==0);
  assert(value==0xfffff000u);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,0x10,0x90000000u)==0);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,4,2)==0);
  assert(ppcvm_pci_bus_decode_memory(&bus,0x90000004u,&hit)==0);
  assert(hit.bus==0 && hit.device==2 && hit.function==0);
  assert(hit.bar_index==0 && hit.offset==4);
  assert(ppcvm_pci_bus_write32(&bus,0,2,0,4,0)==0);
  assert(ppcvm_pci_bus_decode_memory(&bus,0x90000004u,&hit)==1);
  return 0;
}
