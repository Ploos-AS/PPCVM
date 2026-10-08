#include "ppcvm/pegasos2.h"
#include "ppcvm/pci.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { uint32_t value; unsigned reads,writes; int corrupt; } device_state;
static int rd(void *ctx,uint8_t bar,uint64_t off,uint32_t *out) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  ++s->reads;
  *out=s->value ^ (s->corrupt ? 1u : 0u);
  return 0;
}
static int wr(void *ctx,uint8_t bar,uint64_t off,uint32_t value) {
  device_state *s=(device_state *)ctx;
  if(bar!=0 || off!=4) return -1;
  ++s->writes;s->value=value;
  return 0;
}
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  FILE *fp=fopen(argv[1],"rb");
  if(!fp) return 3;
  if(fseek(fp,0,SEEK_END)!=0) {fclose(fp);return 4;}
  long len=ftell(fp);
  if(len<=0 || len>1048576 || fseek(fp,0,SEEK_SET)!=0) {fclose(fp);return 5;}
  uint8_t *elf=malloc((size_t)len);
  if(!elf) {fclose(fp);return 6;}
  if(fread(elf,1,(size_t)len,fp)!=(size_t)len) {fclose(fp);free(elf);return 7;}
  fclose(fp);
  ppcvm_pegasos2 m;
  ppcvm_pci_bus pci;
  ppcvm_pci_device dev;
  ppcvm_pci_mmio_aperture aperture;
  device_state state={0};
  assert(ppcvm_pegasos2_init(&m,4096)==PPCVM_OK);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&dev,0x1234,0x5678,0,0,0);
  assert(ppcvm_pci_set_mem_bar32(&dev,0,4096,0x90000000)==0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&dev)==0);
  assert(ppcvm_pci_bus_set_mmio(&pci,0,2,0,&state,rd,wr)==0);
  assert(ppcvm_pci_map_mmio_aperture(&m.bus,&aperture,&pci,0x90000000,4096)==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,(size_t)len)==PPCVM_OK);
  free(elf);
  assert(m.cpu.pc==0x100u);
  assert(ppcvm_pci_bus_write32(&pci,0,2,0,4,2)==0);
  for(int corrupt=0;corrupt<=1;corrupt++) {
    m.cpu.pc=0x100u;
    m.cpu.gpr[3]=0x90000000u;
    m.cpu.gpr[4]=0x13579bdfu;
    m.cpu.gpr[5]=m.cpu.gpr[6]=m.cpu.gpr[7]=0;
    state.corrupt=corrupt;
    ppcvm_run_report report=ppcvm_cpu_run_bus_diagnostic(&m.cpu,&m.bus,32);
    assert(report.reason==PPCVM_RUN_HALT);
    assert(m.cpu.gpr[6]==(uint32_t)!corrupt);
    assert(state.writes==(unsigned)(corrupt+1));
    assert(state.reads==(unsigned)(corrupt+1));
  }
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
