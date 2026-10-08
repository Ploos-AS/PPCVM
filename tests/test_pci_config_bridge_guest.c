#include "ppcvm/pegasos2.h"
#include "ppcvm/pci_config_bridge.h"
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
  ppcvm_pegasos2 machine;
  ppcvm_pci_bus pci;
  ppcvm_pci_device device;
  ppcvm_pci_config_bridge bridge;
  assert(ppcvm_pegasos2_init(&machine,4096)==PPCVM_OK);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  assert(ppcvm_pci_map_config_bridge(&machine.bus,&bridge,&pci,0x80000000u)==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_boot_elf32(&machine,elf,(size_t)length)==PPCVM_OK);
  free(elf);
  machine.cpu.gpr[3]=0x80000000u;
  ppcvm_run_report report=ppcvm_cpu_run_bus_diagnostic(&machine.cpu,&machine.bus,64);
  assert(report.reason==PPCVM_RUN_HALT);
  assert(machine.cpu.gpr[6]==1u);
  assert(bridge.address==0x80001000u);
  /* Change identity: the same guest must detect the mismatch. */
  pci.slots[0].config.config[0]=0x35u;
  machine.cpu.pc=0x100u;
  machine.cpu.gpr[6]=0;
  report=ppcvm_cpu_run_bus_diagnostic(&machine.cpu,&machine.bus,64);
  assert(report.reason==PPCVM_RUN_HALT);
  assert(machine.cpu.gpr[6]==0u);
  ppcvm_pegasos2_destroy(&machine);
  return 0;
}
