#include "ppcvm/pci.h"
#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
typedef struct { uint32_t reg; unsigned reads,writes; } device_state;
static int rd(void *ctx,uint8_t bar,uint64_t off,uint32_t *value) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->reads++;*value=s->reg;return 0;
}
static int wr(void *ctx,uint8_t bar,uint64_t off,uint32_t value) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->writes++;s->reg=value;return 0;
}
int main(void) {
  ppcvm_bus bus;
  ppcvm_pci_bus pci;
  ppcvm_pci_device dev;
  ppcvm_pci_mmio_aperture aperture;
  ppcvm_cpu cpu;
  device_state state={0};
  ppcvm_bus_init(&bus);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,rd,wr)==0);
  assert(ppcvm_pci_map_mmio_aperture(&bus,&aperture,&pci,0x90000000,4096)==PPCVM_BUS_OK);
  ppcvm_cpu_reset(&cpu);
  cpu.gpr[3]=0x90000000;
  cpu.gpr[4]=0x12345678;
  /* stw r4,4(r3) and lwz r5,4(r3): real PowerPC D-form opcodes. */
  uint32_t stw=(36u<<26)|(4u<<21)|(3u<<16)|4u;
  uint32_t lwz=(32u<<26)|(5u<<21)|(3u<<16)|4u;
  assert(ppcvm_cpu_step_bus(&cpu,&bus,stw)==PPCVM_MEMORY_FAULT);
  assert(state.writes==0);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2)==0);
  assert(ppcvm_cpu_step_bus(&cpu,&bus,stw)==PPCVM_OK);
  assert(state.writes==1 && state.reg==0x12345678);
  assert(ppcvm_cpu_step_bus(&cpu,&bus,lwz)==PPCVM_OK);
  assert(state.reads==1 && cpu.gpr[5]==0x12345678);
  assert(cpu.pc==8);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,0)==0);
  assert(ppcvm_cpu_step_bus(&cpu,&bus,lwz)==PPCVM_MEMORY_FAULT);
  assert(state.reads==1);
  return 0;
}
