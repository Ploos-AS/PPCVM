#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void write_bytes(const char *path,const uint8_t *data,size_t n) {
  FILE *f=fopen(path,"wb");
  assert(f!=NULL);
  assert(fwrite(data,1,n,f)==n);
  assert(fclose(f)==0);
}
int main(void) {
  const char *path="pci-event-snapshot-test.pevt";
  ppcvm_pci_bus pci;
  ppcvm_pci_device config;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device event;
  uint8_t bytes[PPCVM_PCI_EVENT_WIRE_SIZE+1u];
  FILE *f;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&config,0x1234,0x5681,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&config)==0);
  ppcvm_discovery_ii_init(&controller);
  ppcvm_discovery_ii_enable_irq_candidate(&controller);
  controller.irq_cpu0_mask_low=1u;
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&event,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_write32(&event,0,12,3u)==0);
  assert(ppcvm_pci_event_device_write32(&event,0,4,1u)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_save_file(&event,path)==0);
  f=fopen(path,"rb");assert(f!=NULL);
  assert(fread(bytes,1,16u,f)==16u);
  assert(fgetc(f)==EOF);assert(fclose(f)==0);
  assert(memcmp(bytes,"PEVT",4u)==0);
  assert(ppcvm_pci_event_device_reset(&event)==0);
  assert(ppcvm_pci_event_device_load_file(&event,path)==0);
  assert(event.pending==3u && event.enable==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* Failed loads must not modify the live device or IRQ. */
  assert(ppcvm_pci_event_device_load_file(&event,"missing-pevt-file")==-1);
  write_bytes(path,bytes,15u);
  assert(ppcvm_pci_event_device_load_file(&event,path)==-1);
  bytes[16]=0u;write_bytes(path,bytes,17u);
  assert(ppcvm_pci_event_device_load_file(&event,path)==-1);
  bytes[0]='X';write_bytes(path,bytes,16u);
  assert(ppcvm_pci_event_device_load_file(&event,path)==-1);
  bytes[0]='P';bytes[7]=2u;write_bytes(path,bytes,16u);
  assert(ppcvm_pci_event_device_load_file(&event,path)==-1);
  bytes[7]=1u;bytes[11]=16u;write_bytes(path,bytes,16u);
  assert(ppcvm_pci_event_device_load_file(&event,path)==-1);
  assert(event.pending==3u && event.enable==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  assert(ppcvm_pci_event_device_load_file(&event,0)==-1);
  assert(ppcvm_pci_event_device_save_file(&event,0)==-1);
  /* Replacing an existing file must publish the new complete frame. */
  assert(ppcvm_pci_event_device_write32(&event,0,8,1u)==0);
  assert(ppcvm_pci_event_device_write32(&event,0,4,2u)==0);
  assert(ppcvm_pci_event_device_save_file(&event,path)==0);
  assert(ppcvm_pci_event_device_reset(&event)==0);
  assert(ppcvm_pci_event_device_load_file(&event,path)==0);
  assert(event.pending==2u && event.enable==2u);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==1u);
  /* A failed staging open must leave the published snapshot untouched. */
  assert(ppcvm_pci_event_device_save_file(&event,".")==-1);
  assert(ppcvm_pci_event_device_load_file(&event,path)==0);
  assert(event.pending==2u && event.enable==2u);
  assert(ppcvm_pci_event_device_reset(&event)==0);
  assert(ppcvm_discovery_ii_active_irq_low(&controller)==0u);
  assert(remove(path)==0);
  return 0;
}
