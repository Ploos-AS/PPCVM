#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
typedef struct { uint32_t value; unsigned reads,writes; } state;
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
  ppcvm_pci_bus b;
  ppcvm_pci_device d;
  state st={0};
  uint32_t value=0;
  ppcvm_pci_bus_init(&b);
  ppcvm_pci_device_init(&d,1,2,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&d,0,4096,0x80000000)==0);
  assert(ppcvm_pci_bus_add(&b,0,3,0,&d)==0);
  assert(ppcvm_pci_bus_set_mmio(&b,0,3,0,&st,rd,wr)==0);
  assert(ppcvm_pci_bus_mmio_write32(&b,0x80000004,123)!=0);
  assert(ppcvm_pci_bus_write32(&b,0,3,0,4,2)==0);
  assert(ppcvm_pci_bus_mmio_write32(&b,0x80000004,123)==0);
  assert(ppcvm_pci_bus_mmio_read32(&b,0x80000004,&value)==0 && value==123);
  assert(st.reads==1 && st.writes==1);
  assert(ppcvm_pci_bus_mmio_read32(&b,0x80000003,&value)!=0);
  assert(ppcvm_pci_bus_mmio_read32(&b,0x80000008,&value)!=0);
  assert(ppcvm_pci_bus_write32(&b,0,3,0,4,0)==0);
  assert(ppcvm_pci_bus_mmio_read32(&b,0x80000004,&value)!=0);
  return 0;
}
