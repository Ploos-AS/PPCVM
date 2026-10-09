#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
typedef struct {uint32_t value;unsigned reads,writes;} state;
static int rd(void *ctx,uint8_t bar,uint64_t off,uint32_t *v) {
  state *s=(state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->reads++;*v=s->value;return 0;
}
static int wr(void *ctx,uint8_t bar,uint64_t off,uint32_t v) {
  state *s=(state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->writes++;s->value=v;return 0;
}
int main(void) {
  ppcvm_bus bus;ppcvm_pci_bus pci;ppcvm_pci_device dev;
  ppcvm_pci_translated_window window;
  state s={0};uint32_t value=0;
  ppcvm_bus_init(&bus);ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000u)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&s,rd,wr)==0);
  assert(ppcvm_pci_map_translated_window(&bus,&window,&pci,
      0xa0000000u,4096,0x90000000u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0xa0000004u,&value)==PPCVM_BUS_UNMAPPED);
  ppcvm_pci_translated_window_enable(&window,1);
  assert(ppcvm_bus_write32be(&bus,0xa0000004u,0x13579bdfu)==PPCVM_BUS_UNMAPPED);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2u)==0);
  assert(ppcvm_bus_write32be(&bus,0xa0000004u,0x13579bdfu)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&bus,0xa0000004u,&value)==PPCVM_BUS_OK);
  assert(value==0x13579bdfu && s.reads==1 && s.writes==1);
  ppcvm_pci_translated_window_enable(&window,0);
  assert(ppcvm_bus_read32be(&bus,0xa0000004u,&value)==PPCVM_BUS_UNMAPPED);
  assert(s.reads==1 && s.writes==1);
  assert(ppcvm_pci_map_translated_window(&bus,&window,&pci,
      0xfffff000u,8192,0x90000000u)==PPCVM_BUS_INVALID);
  return 0;
}
