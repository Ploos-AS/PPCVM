#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_device d;
  uint32_t v=0;
  ppcvm_pci_device_init(&d,0x1234,0x5678,2,0,0);
  assert(ppcvm_pci_set_mem_bar64(&d,0,UINT64_C(0x200000000),UINT64_C(0x400000000))==0);
  assert(ppcvm_pci_set_mem_bar32(&d,1,4096,0)!=0);
  assert(ppcvm_pci_set_io_bar32(&d,0,256,0)!=0);
  assert(ppcvm_pci_set_mem_bar64(&d,5,4096,0)!=0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==4u);
  assert(ppcvm_pci_read32(&d,0x14,&v)==0 && v==4u);
  assert(ppcvm_pci_write32(&d,0x10,UINT32_MAX)==0);
  assert(ppcvm_pci_write32(&d,0x14,UINT32_MAX)==0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==4u);
  assert(ppcvm_pci_read32(&d,0x14,&v)==0 && v==0xfffffffeu);
  assert(ppcvm_pci_write32(&d,0x14,6u)==0);
  assert(ppcvm_pci_read32(&d,0x14,&v)==0 && v==6u);
  assert(ppcvm_pci_write32(&d,0x10,0u)==0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==4u);
  return 0;
}
