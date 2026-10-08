#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus b;
  ppcvm_pci_device d;
  uint32_t v=0;
  ppcvm_pci_bus_init(&b);
  ppcvm_pci_device_init(&d,0x1234,0x5678,6,0,1);
  assert(ppcvm_pci_bus_add(&b,0,3,0,&d)==0);
  assert(ppcvm_pci_bus_add(&b,0,3,0,&d)!=0);
  assert(ppcvm_pci_bus_read32(&b,0,3,0,0,&v)==0 && v==0x56781234);
  assert(ppcvm_pci_bus_read32(&b,0,4,0,0,&v)==0 && v==UINT32_MAX);
  assert(ppcvm_pci_bus_write32(&b,0,4,0,0x10,0xabcdef01)==0);
  assert(ppcvm_pci_bus_write32(&b,0,3,0,0x10,0xabcdef01)==0);
  assert(ppcvm_pci_bus_read32(&b,0,3,0,0x10,&v)==0 && v==0xabcdef01);
  assert(ppcvm_pci_bus_read32(&b,1,3,0,0,&v)==0 && v==UINT32_MAX);
  assert(ppcvm_pci_bus_add(&b,0,32,0,&d)!=0);
  assert(ppcvm_pci_bus_add(&b,0,0,8,&d)!=0);
  assert(ppcvm_pci_bus_read32(&b,0,3,0,255,&v)!=0);
  return 0;
}
