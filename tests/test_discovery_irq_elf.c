#include "ppcvm/pegasos2.h"
#include "ppcvm/pci_irq_bridge.h"
#include "ppcvm/pci_irq_matrix.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  FILE *fp=fopen(argv[1],"rb");
  if(!fp) return 3;
  if(fseek(fp,0,SEEK_END)!=0) {fclose(fp);return 4;}
  long len=ftell(fp);
  if(len<=0 || len>1048576L || fseek(fp,0,SEEK_SET)!=0) {fclose(fp);return 5;}
  uint8_t *elf=malloc((size_t)len);
  if(!elf) {fclose(fp);return 6;}
  if(fread(elf,1,(size_t)len,fp)!=(size_t)len) {free(elf);fclose(fp);return 7;}
  fclose(fp);
  ppcvm_pegasos2 m;
  ppcvm_pci_bus pci;
  ppcvm_pci_device device,device2,device3;
  ppcvm_pci_irq_matrix matrix;
  assert(ppcvm_pegasos2_init(&m,65536u)==0);
  assert(ppcvm_pegasos2_map_discovery_ii(&m,UINT32_C(0x20000),0x100u)==PPCVM_BUS_OK);
  ppcvm_discovery_ii_enable_irq_candidate(&m.discovery_ii);
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  ppcvm_pci_device_init(&device2,0x1234,0x5679,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,3,0,&device2)==0);
  ppcvm_pci_device_init(&device3,0x1234,0x5680,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,4,0,&device3)==0);
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&m.discovery_ii)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,3,0,1,0)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,4,0,1,1)==0);
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,(size_t)len)==PPCVM_OK);
  free(elf);
  assert(m.cpu.pc==0x1000u);
  m.cpu.msr=UINT32_C(0x8000);
  /* Guest must enable mask itself before host asserts the source. */
  for(unsigned i=0;i<3;i++) assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==0);
  assert(m.discovery_ii.irq_cpu0_mask_low==1u);
  /* Two PCI devices share source 0; a third asserts independent source 1. */
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,4,0,1,1)==0);
  assert(m.discovery_ii.irq_asserted_low==2u);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==0u);
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,2,0,1,1)==0);
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,3,0,1,1)==0);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL);
  for(unsigned i=0;i<6;i++) assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.gpr[7]==UINT32_C(0x03000000)); /* bus-visible cause bits 0 and 1 */
  assert(m.cpu.gpr[6]==UINT32_C(0x50415353));
  assert(m.discovery_ii.irq_cpu0_mask_low==0);
  assert(m.discovery_ii.irq_asserted_low==3u);
  assert(m.cpu.pc!=PPCVM_VECTOR_EXTERNAL);
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,2,0,1,0)==0);
  assert(m.discovery_ii.irq_asserted_low==3u);
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,3,0,1,0)==0);
  assert(m.discovery_ii.irq_asserted_low==2u);
  assert(ppcvm_pci_irq_matrix_set_level(&matrix,0,4,0,1,0)==0);
  assert(m.discovery_ii.irq_asserted_low==0u);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
