#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_device d;
  ppcvm_pci_device_init(&d,0x1234,0x5678,1,0,0);
  uint32_t v=0;
  assert(ppcvm_pci_set_io_bar32(&d,0,256,0x1000)==0);
  assert(ppcvm_pci_set_mem_bar32(&d,1,4096,0x80000000)==0);
  assert(ppcvm_pci_set_io_bar32(&d,2,3,0)!=0);
  assert(ppcvm_pci_set_io_bar32(&d,2,8,3)!=0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==0x1001);
  assert(ppcvm_pci_write32(&d,0x10,UINT32_MAX)==0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==0xffffff01);
  assert(ppcvm_pci_read32(&d,0x14,&v)==0 && v==0x80000000);
  assert(ppcvm_pci_write32(&d,0x10,0x12345)==0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==0x12301);
  assert(ppcvm_pci_write32(&d,0x14,UINT32_MAX)==0);
  assert(ppcvm_pci_read32(&d,0x14,&v)==0 && v==0xfffff000);
  return 0;
}
