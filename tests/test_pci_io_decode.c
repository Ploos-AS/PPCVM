#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus b;
  ppcvm_pci_device d;
  ppcvm_pci_bar_hit hit={0};
  ppcvm_pci_bus_init(&b);
  ppcvm_pci_device_init(&d,1,2,0,0,0);
  assert(ppcvm_pci_set_io_bar32(&d,0,256,0x1000)==0);
  assert(ppcvm_pci_set_mem_bar32(&d,1,4096,0x80000000)==0);
  assert(ppcvm_pci_bus_add(&b,0,2,0,&d)==0);
  assert(ppcvm_pci_bus_decode_io(&b,0x1020,&hit)==1);
  assert(ppcvm_pci_bus_write32(&b,0,2,0,4,2)==0);
  assert(ppcvm_pci_bus_decode_io(&b,0x1020,&hit)==1);
  assert(ppcvm_pci_bus_write32(&b,0,2,0,4,1)==0);
  assert(ppcvm_pci_bus_decode_io(&b,0x1020,&hit)==0);
  assert(hit.device==2 && hit.bar_index==0 && hit.offset==0x20);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000020,&hit)==1);
  assert(ppcvm_pci_bus_decode_io(&b,0x1100,&hit)==1);
  ppcvm_pci_device_init(&d,3,4,0,0,0);
  assert(ppcvm_pci_set_io_bar32(&d,0,256,0x1000)==0);
  assert(ppcvm_pci_write32(&d,4,1)==0);
  assert(ppcvm_pci_bus_add(&b,0,3,0,&d)==0);
  assert(ppcvm_pci_bus_decode_io(&b,0x1020,&hit)==-1);
  return 0;
}
