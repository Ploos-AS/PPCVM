#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pci_bus b;
  ppcvm_pci_device d;
  ppcvm_pci_bar_hit hit={0};
  uint32_t value=0;
  ppcvm_pci_bus_init(&b);
  ppcvm_pci_device_init(&d,1,2,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&d,0,4096,0x80000000)==0);
  assert(ppcvm_pci_bus_add(&b,0,1,0,&d)==0);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000010,&hit)==1);
  assert(ppcvm_pci_bus_write32(&b,0,1,0,4,2)==0);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000010,&hit)==0);
  assert(hit.device==1 && hit.offset==16);
  assert(ppcvm_pci_bus_read32(&b,0,1,0,4,&value)==0 && value==2);
  assert(ppcvm_pci_bus_write32(&b,0,1,0,4,1)==0);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000010,&hit)==1);
  assert(ppcvm_pci_bus_write32(&b,0,1,0,4,0xffffffff)==0);
  assert(ppcvm_pci_bus_read32(&b,0,1,0,4,&value)==0 && value==7);
  assert(ppcvm_pci_bus_decode_memory(&b,0x80000010,&hit)==0);
  return 0;
}
