#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif
int main(void) {
#ifndef _WIN32
  const char *path="pci-event-concurrent.pevt";
  ppcvm_pci_bus pci;
  ppcvm_pci_device config;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device event;
  uint8_t frame[PPCVM_PCI_EVENT_WIRE_SIZE];
  FILE *f;
  pid_t child;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&config,0x1234,0x5681,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&config)==0);
  ppcvm_discovery_ii_init(&controller);
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&event,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_write32(&event,0,12,1u)==0);
  child=fork();
  assert(child>=0);
  if(child==0) {
    for(unsigned i=0;i<32;i++)
      if(ppcvm_pci_event_device_save_file(&event,path)!=0) _exit(1);
    _exit(0);
  }
  assert(ppcvm_pci_event_device_write32(&event,0,12,2u)==0);
  for(unsigned i=0;i<32;i++)
    assert(ppcvm_pci_event_device_save_file(&event,path)==0);
  {
    int status=0;
    assert(waitpid(child,&status,0)==child);
    assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
  }
  f=fopen(path,"rb");assert(f!=NULL);
  assert(fread(frame,1,sizeof(frame),f)==sizeof(frame));
  assert(fgetc(f)==EOF);assert(fclose(f)==0);
  {
    ppcvm_pci_event_snapshot snapshot;
    assert(ppcvm_pci_event_snapshot_decode(&snapshot,frame,sizeof(frame))==0);
    assert(snapshot.pending==1u || snapshot.pending==3u);
  }
  assert(remove(path)==0);
#endif
  return 0;
}
