#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus b;
  ppcvm_pci_device d;
  ppcvm_pci_bar_hit hit={0};
  ppcvm_pci_bus_init(&b);
  ppcvm_pci_device_init(&d,1,2,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&d,0,4096,0x80000000)==0);
  assert(ppcvm_pci_set_io_bar32(&d,1,256,0x1000)==0);
  assert(ppcvm_pci_set_mem_bar64(&d,2,UINT64_C(0x200000000),UINT64_C(0x400000000))==0);
  assert(ppcvm_pci_bus_add(&b,0,2,0,&d)==0);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000123,&hit)==0);
  assert(hit.device==2 && hit.bar_index==0 && hit.offset==0x123);
  assert(ppcvm_pci_bus_decode_memory(&b,0x1000,&hit)==1);
  assert(ppcvm_pci_bus_decode_memory(&b,UINT64_C(0x400000100),&hit)==0);
  assert(hit.bar_index==2 && hit.offset==0x100);
  assert(ppcvm_pci_bus_decode_memory(&b,UINT64_C(0x600000000),&hit)==1);
  ppcvm_pci_device_init(&d,3,4,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&d,0,4096,0x80000000)==0);
  assert(ppcvm_pci_bus_add(&b,0,3,0,&d)==0);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000000,&hit)==-1);
  assert(ppcvm_pci_bus_decode_memory(&b,0x90000000,0)==-1);
  return 0;
}
