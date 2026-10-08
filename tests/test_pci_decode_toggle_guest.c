#include "ppcvm/pegasos2.h"
#include "ppcvm/pci_config_bridge.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {uint32_t value;unsigned reads,writes;} device_state;
static int rd(void *ctx,uint8_t bar,uint64_t off,uint32_t *value) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->reads++;*value=s->value;return 0;
}
static int wr(void *ctx,uint8_t bar,uint64_t off,uint32_t value) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  s->writes++;s->value=value;return 0;
}
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  FILE *fp=fopen(argv[1],"rb");
  if(!fp) return 3;
  if(fseek(fp,0,SEEK_END)!=0) {fclose(fp);return 4;}
  long length=ftell(fp);
  if(length<=0 || length>1048576L || fseek(fp,0,SEEK_SET)!=0) {fclose(fp);return 5;}
  uint8_t *elf=malloc((size_t)length);
  if(!elf) {fclose(fp);return 6;}
  if(fread(elf,1,(size_t)length,fp)!=(size_t)length) {free(elf);fclose(fp);return 7;}
  fclose(fp);
  ppcvm_pegasos2 machine;
  ppcvm_pci_bus pci;
  ppcvm_pci_device device;
  ppcvm_pci_config_bridge bridge;
  ppcvm_pci_mmio_aperture aperture;
  device_state state={0};
  assert(ppcvm_pegasos2_init(&machine,4096)==PPCVM_OK);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_set_mem_bar32(&device,0,4096,0x90000000u)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,rd,wr)==0);
  assert(ppcvm_pci_map_config_bridge(&machine.bus,&bridge,&pci,0x80000000u)==PPCVM_BUS_OK);
  assert(ppcvm_pci_map_mmio_aperture(&machine.bus,&aperture,&pci,0x90000000u,4096)==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_boot_elf32(&machine,elf,(size_t)length)==PPCVM_OK);
  free(elf);
  uint32_t value=0;
  assert(ppcvm_bus_read32be(&machine.bus,0x90000004u,&value)==PPCVM_BUS_UNMAPPED);
  machine.cpu.gpr[3]=0x80000000u;
  machine.cpu.gpr[4]=0x90000000u;
  machine.cpu.gpr[5]=0x2468ace0u;
  ppcvm_run_report report=ppcvm_cpu_run_bus_diagnostic(&machine.cpu,&machine.bus,128);
  assert(report.reason==PPCVM_RUN_HALT);
  assert(machine.cpu.gpr[6]==1u);
  assert(state.value==0x2468ace0u && state.reads==1 && state.writes==1);
  assert(ppcvm_pci_bus_read32(&pci,0,2,0,4,&value)==0);
  assert((value&2u)==0);
  assert(ppcvm_bus_read32be(&machine.bus,0x90000004u,&value)==PPCVM_BUS_UNMAPPED);
  assert(ppcvm_bus_write32be(&machine.bus,0x90000004u,0xffffffffu)==PPCVM_BUS_UNMAPPED);
  assert(state.reads==1 && state.writes==1);
  ppcvm_pegasos2_destroy(&machine);
  return 0;
}
