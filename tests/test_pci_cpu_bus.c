#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
typedef struct { uint32_t value; } device_state;
static int read_cb(void *ctx,uint8_t bar,uint64_t off,uint32_t *value) {
  if(bar!=0 || off!=4) return -1;
  *value=((device_state *)ctx)->value;return 0;
}
static int write_cb(void *ctx,uint8_t bar,uint64_t off,uint32_t value) {
  if(bar!=0 || off!=4) return -1;
  ((device_state *)ctx)->value=value;return 0;
}
int main(void) {
  ppcvm_bus cpu;
  ppcvm_pci_bus pci;
  ppcvm_pci_device dev;
  ppcvm_pci_mmio_aperture aperture;
  device_state state={0};
  uint32_t value=0;
  ppcvm_bus_init(&cpu);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,1,2,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,read_cb,write_cb)==0);
  assert(ppcvm_pci_map_mmio_aperture(&cpu,&aperture,&pci,0x90000000,4096)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&cpu,0x90000004,0x12345678)!=PPCVM_BUS_OK);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2)==0);
  assert(ppcvm_bus_write32be(&cpu,0x90000004,0x12345678)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&cpu,0x90000004,&value)==PPCVM_BUS_OK && value==0x12345678);
  assert(ppcvm_bus_read32be(&cpu,0x90000008,&value)!=PPCVM_BUS_OK);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,0)==0);
  assert(ppcvm_bus_read32be(&cpu,0x90000004,&value)!=PPCVM_BUS_OK);
  return 0;
}
