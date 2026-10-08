#include "ppcvm/pegasos2.h"
#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef struct {uint32_t value;unsigned reads,writes;} device_state;
static int rd(void *ctx,uint8_t bar,uint64_t off,uint32_t *value) {
  device_state *s=(device_state *)ctx;
  if(bar || off!=4) return -1;
  s->reads++;*value=s->value;return 0;
}
static int wr(void *ctx,uint8_t bar,uint64_t off,uint32_t value) {
  device_state *s=(device_state *)ctx;
  if(bar || off!=4) return -1;
  s->writes++;s->value=value;return 0;
}
static void be16(uint8_t *p,uint16_t x) {p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void be32(uint8_t *p,uint32_t x) {
  p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;
}
int main(void) {
  ppcvm_pegasos2 m;
  ppcvm_pci_bus pci;
  ppcvm_pci_device dev;
  ppcvm_pci_mmio_aperture aperture;
  device_state state={0};
  uint8_t elf[0x110]={0};
  const uint32_t entry=0x100u;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,rd,wr)==0);
  assert(ppcvm_pci_map_mmio_aperture(&m.bus,&aperture,&pci,0x90000000,4096)==PPCVM_BUS_OK);
  memcpy(elf,"\177ELF",4);
  elf[4]=1;elf[5]=2;elf[6]=1;
  be16(elf+16,2);be16(elf+18,20);be32(elf+20,1);
  be32(elf+24,entry);be32(elf+28,52);
  be16(elf+40,52);be16(elf+42,32);be16(elf+44,1);
  be32(elf+52,1);be32(elf+56,0x100);be32(elf+60,entry);
  be32(elf+64,entry);be32(elf+68,16);be32(elf+72,16);
  be32(elf+76,5);be32(elf+80,4);
  be32(elf+0x100,(36u<<26)|(4u<<21)|(3u<<16)|4u);
  be32(elf+0x104,(32u<<26)|(5u<<21)|(3u<<16)|4u);
  be32(elf+0x108,(24u<<26)|(5u<<21)|(6u<<16));
  be32(elf+0x10c,PPCVM_DIAGNOSTIC_HALT);
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,sizeof(elf))==PPCVM_OK);
  assert(m.cpu.pc==entry);
  m.cpu.gpr[3]=0x90000000;
  m.cpu.gpr[4]=0x13579bdf;
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2)==0);
  ppcvm_run_report report=ppcvm_cpu_run_bus_diagnostic(&m.cpu,&m.bus,16);
  assert(report.reason==PPCVM_RUN_HALT && report.executed==3);
  assert(report.final_pc==entry+12);
  assert(m.cpu.gpr[5]==0x13579bdf && m.cpu.gpr[6]==0x13579bdf);
  assert(state.value==0x13579bdf && state.reads==1 && state.writes==1);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
