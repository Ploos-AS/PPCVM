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
  uint8_t ram[256]={0};
  uint32_t instruction=0;
  ppcvm_bus_init(&bus);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,rd,wr)==0);
  assert(ppcvm_bus_map_memory(&bus,0,sizeof(ram),ram,0)==PPCVM_BUS_OK);
  assert(ppcvm_pci_map_mmio_aperture(&bus,&aperture,&pci,0x90000000,4096)==PPCVM_BUS_OK);
  /* Program: stw r4,4(r3); lwz r5,4(r3); ori r6,r5,0. */
  assert(ppcvm_bus_write32be(&bus,0,(36u<<26)|(4u<<21)|(3u<<16)|4u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,4,(32u<<26)|(5u<<21)|(3u<<16)|4u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&bus,8,(24u<<26)|(5u<<21)|(6u<<16))==PPCVM_BUS_OK);
  ppcvm_cpu_reset(&cpu);
  cpu.gpr[3]=0x90000000;
  cpu.gpr[4]=0xabcdef12;
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2)==0);
  for(unsigned i=0;i<3;i++) {
    assert(ppcvm_bus_read32be(&bus,cpu.pc,&instruction)==PPCVM_BUS_OK);
    assert(ppcvm_cpu_step_bus(&cpu,&bus,instruction)==PPCVM_OK);
  }
  assert(cpu.pc==12);
  assert(cpu.gpr[5]==0xabcdef12 && cpu.gpr[6]==0xabcdef12);
  assert(state.reads==1 && state.writes==1);
  return 0;
}
