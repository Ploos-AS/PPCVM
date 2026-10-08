#include "ppcvm/pegasos2.h"
#include "ppcvm/discovery_ii.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
  ppcvm_pegasos2 m;
  ppcvm_pci_bus pci,pci1;
  ppcvm_pci_device device,device1;
  assert(ppcvm_pegasos2_init(&m,4096)==PPCVM_OK);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_bus_init(&pci1);
  ppcvm_pci_device_init(&device,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  ppcvm_pci_device_init(&device1,0xabcd,0xef01,2,0,1);
  assert(ppcvm_pci_bus_add(&pci1,0,2,0,&device1)==0);
  assert(ppcvm_pegasos2_map_discovery_ii(&m,0x80000000u,4096)==PPCVM_BUS_OK);
  ppcvm_discovery_ii_enable_pci_config(&m.discovery_ii,&pci,&pci1);
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,(size_t)length)==PPCVM_OK);
  free(elf);
  m.cpu.gpr[3]=0x80000000u;
  ppcvm_run_report report=ppcvm_cpu_run_bus_diagnostic(&m.cpu,&m.bus,64);
  assert(report.reason==PPCVM_RUN_HALT);
  assert(m.cpu.gpr[6]==1u);
  assert(m.discovery_ii.config_address[0]==0x80001000u);
  assert(m.discovery_ii.config_address[1]==0x80001000u);
  pci.slots[0].config.config[0]=0x35u;
  m.cpu.pc=0x100u;
  m.cpu.gpr[6]=0;
  report=ppcvm_cpu_run_bus_diagnostic(&m.cpu,&m.bus,64);
  assert(report.reason==PPCVM_RUN_HALT);
  assert(m.cpu.gpr[6]==0u);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
