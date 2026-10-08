#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_device d;
  uint32_t v=0;
  ppcvm_pci_device_init(&d,0x1234,0xabcd,0x06,0x00,0x12);
  assert(ppcvm_pci_read32(&d,0,&v)==0 && v==0xabcd1234);
  assert(ppcvm_pci_read32(&d,8,&v)==0 && v==0x06000012);
  assert(ppcvm_pci_write32(&d,0,0xffffffff)==0);
  assert(ppcvm_pci_read32(&d,0,&v)==0 && v==0xabcd1234);
  assert(ppcvm_pci_write32(&d,0x10,0xdeadbeef)==0);
  assert(ppcvm_pci_read32(&d,0x10,&v)==0 && v==0xdeadbeef);
  assert(ppcvm_pci_read32(&d,253,&v)!=0);
  assert(ppcvm_pci_write32(&d,3,1)!=0);
  return 0;
}
