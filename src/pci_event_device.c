#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "ppcvm/pci_event_device.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#define EVENT_BITS UINT32_C(0x0f)
static int apply(ppcvm_pci_event_device *d,uint32_t pending,uint32_t enable) {
  if(ppcvm_pci_irq_matrix_set_level(d->matrix,d->bus,d->device,
      d->function,d->pin,(pending&enable)!=0u)!=0) return -1;
  d->pending=pending;d->enable=enable;
  return 0;
}
int ppcvm_pci_event_device_init(ppcvm_pci_event_device *d,
    ppcvm_pci_irq_matrix *matrix,uint8_t bus,uint8_t device,
    uint8_t function,uint8_t pin) {
  if(!d || !matrix || pin<1 || pin>4) return -1;
  d->matrix=matrix;d->bus=bus;d->device=device;d->function=function;
  d->pin=pin;d->pending=0;d->enable=0;
  return 0;
}
int ppcvm_pci_event_device_reset(ppcvm_pci_event_device *d) {
  if(!d || !d->matrix) return -1;
  return apply(d,0u,0u);
}
int ppcvm_pci_event_device_save(const ppcvm_pci_event_device *d,
    ppcvm_pci_event_snapshot *snapshot) {
  if(!d || !d->matrix || !snapshot) return -1;
  snapshot->version=PPCVM_PCI_EVENT_SNAPSHOT_VERSION;
  snapshot->pending=d->pending;
  snapshot->enable=d->enable;
  return 0;
}
int ppcvm_pci_event_device_restore(ppcvm_pci_event_device *d,
    const ppcvm_pci_event_snapshot *snapshot) {
  if(!d || !d->matrix || !snapshot ||
      snapshot->version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((snapshot->pending|snapshot->enable)&~EVENT_BITS)!=0u) return -1;
  return apply(d,snapshot->pending,snapshot->enable);
}
static void put_be32(uint8_t *p,uint32_t v) {
  p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);
  p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;
}
static uint32_t get_be32(const uint8_t *p) {
  return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
      ((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
int ppcvm_pci_event_snapshot_encode(const ppcvm_pci_event_snapshot *snapshot,
    uint8_t *out,size_t size) {
  if(!snapshot || !out || size<PPCVM_PCI_EVENT_WIRE_SIZE ||
      snapshot->version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((snapshot->pending|snapshot->enable)&~EVENT_BITS)!=0u) return -1;
  out[0]='P';out[1]='E';out[2]='V';out[3]='T';
  put_be32(out+4,snapshot->version);
  put_be32(out+8,snapshot->pending);
  put_be32(out+12,snapshot->enable);
  return 0;
}
int ppcvm_pci_event_snapshot_decode(ppcvm_pci_event_snapshot *snapshot,
    const uint8_t *data,size_t size) {
  ppcvm_pci_event_snapshot temp;
  if(!snapshot || !data || size!=PPCVM_PCI_EVENT_WIRE_SIZE ||
      data[0]!='P' || data[1]!='E' || data[2]!='V' || data[3]!='T') return -1;
  temp.version=get_be32(data+4);
  temp.pending=get_be32(data+8);
  temp.enable=get_be32(data+12);
  if(temp.version!=PPCVM_PCI_EVENT_SNAPSHOT_VERSION ||
      ((temp.pending|temp.enable)&~EVENT_BITS)!=0u) return -1;
  *snapshot=temp;
  return 0;
}
int ppcvm_pci_event_device_save_file(const ppcvm_pci_event_device *d,
    const char *path) {
  ppcvm_pci_event_snapshot snapshot;
  uint8_t bytes[PPCVM_PCI_EVENT_WIRE_SIZE];
  FILE *fp;
  char *temp;
  size_t n;
  int ok;
  if(!path || ppcvm_pci_event_device_save(d,&snapshot)!=0 ||
      ppcvm_pci_event_snapshot_encode(&snapshot,bytes,sizeof(bytes))!=0)
    return -1;
  n=strlen(path);
#ifdef _WIN32
  /* Windows fallback remains single-writer; unique staging is POSIX-only. */
  if(n>SIZE_MAX-5u) return -1;
  temp=(char *)malloc(n+5u);
  if(!temp) return -1;
  memcpy(temp,path,n);
  memcpy(temp+n,".tmp",5u);
  fp=fopen(temp,"wb");
  if(!fp) {free(temp);return -1;}
#else
  int fd;
  if(n>SIZE_MAX-12u) return -1;
  temp=(char *)malloc(n+12u);
  if(!temp) return -1;
  memcpy(temp,path,n);
  memcpy(temp+n,".tmp.XXXXXX",12u);
  fd=mkstemp(temp);
  if(fd<0) {free(temp);return -1;}
  fp=fdopen(fd,"wb");
  if(!fp) {close(fd);unlink(temp);free(temp);return -1;}
#endif
  ok=fwrite(bytes,1,sizeof(bytes),fp)==sizeof(bytes);
  if(fflush(fp)!=0) ok=0;
#ifndef _WIN32
  if(ok && fsync(fileno(fp))!=0) ok=0;
#endif
  if(fclose(fp)!=0) ok=0;
  if(ok && rename(temp,path)!=0) ok=0;
#ifndef _WIN32
  /* Sync the containing directory after publication where supported.
   * A sync failure is reported even though rename may have succeeded. */
  if(ok) {
    const char *slash=strrchr(path,'/');
    char *dir;
    int dirfd;
    size_t len=slash?(size_t)(slash-path):1u;
    if(slash && len==0u) len=1u;
    dir=(char *)malloc(len+1u);
    if(!dir) ok=0;
    else {
      if(slash) memcpy(dir,path,len);
      else dir[0]='.';
      dir[len]='\\0';
      dirfd=open(dir,O_RDONLY);
      if(dirfd<0) ok=0;
      else {
        if(fsync(dirfd)!=0) ok=0;
        if(close(dirfd)!=0) ok=0;
      }
      free(dir);
    }
  }
#endif
  if(!ok) remove(temp);
  free(temp);
  return ok?0:-1;
}
int ppcvm_pci_event_device_load_file(ppcvm_pci_event_device *d,
    const char *path) {
  ppcvm_pci_event_snapshot snapshot;
  uint8_t bytes[PPCVM_PCI_EVENT_WIRE_SIZE];
  FILE *fp;
  size_t n;
  int extra,close_ok;
  if(!d || !path) return -1;
  fp=fopen(path,"rb");
  if(!fp) return -1;
  n=fread(bytes,1,sizeof(bytes),fp);
  extra=fgetc(fp);
  close_ok=fclose(fp)==0;
  if(n!=sizeof(bytes) || extra!=EOF || !close_ok ||
      ppcvm_pci_event_snapshot_decode(&snapshot,bytes,sizeof(bytes))!=0)
    return -1;
  return ppcvm_pci_event_device_restore(d,&snapshot);
}
int ppcvm_pci_event_device_read32(void *context,uint8_t bar,uint64_t offset,uint32_t *value) {
  ppcvm_pci_event_device *d=(ppcvm_pci_event_device *)context;
  if(!d || !value || bar!=0) return -1;
  switch(offset) {
    case 0: *value=d->pending;return 0;
    case 4: *value=d->enable;return 0;
    case 8: case 12: *value=0;return 0;
    default:return -1;
  }
}
int ppcvm_pci_event_device_write32(void *context,uint8_t bar,uint64_t offset,uint32_t value) {
  ppcvm_pci_event_device *d=(ppcvm_pci_event_device *)context;
  if(!d || !d->matrix || bar!=0 || (value&~EVENT_BITS)!=0) return -1;
  switch(offset) {
    case 4:return apply(d,d->pending,value);
    case 8:return apply(d,d->pending&~value,d->enable);
    case 12:return apply(d,d->pending|value,d->enable);
    default:return -1;
  }
}
