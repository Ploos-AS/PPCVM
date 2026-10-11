#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
int main(void) {
  ppcvm_pci_event_snapshot s={PPCVM_PCI_EVENT_SNAPSHOT_VERSION,5u,3u};
  ppcvm_pci_event_snapshot decoded={99u,99u,99u};
  uint8_t buf[PPCVM_PCI_EVENT_WIRE_SIZE];
  const uint8_t expected[16]={
    'P','E','V','T',0,0,0,1,0,0,0,5,0,0,0,3
  };
  assert(ppcvm_pci_event_snapshot_encode(&s,buf,sizeof(buf))==0);
  assert(memcmp(buf,expected,sizeof(buf))==0);
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,sizeof(buf))==0);
  assert(decoded.version==1u && decoded.pending==5u && decoded.enable==3u);
  /* No partial writes on invalid encode. */
  memset(buf,0xa5,sizeof(buf));
  assert(ppcvm_pci_event_snapshot_encode(&s,buf,15u)==-1);
  for(unsigned i=0;i<sizeof(buf);i++) assert(buf[i]==0xa5u);
  s.pending=16u;
  assert(ppcvm_pci_event_snapshot_encode(&s,buf,sizeof(buf))==-1);
  for(unsigned i=0;i<sizeof(buf);i++) assert(buf[i]==0xa5u);
  s.pending=5u;s.version=2u;
  assert(ppcvm_pci_event_snapshot_encode(&s,buf,sizeof(buf))==-1);
  s.version=1u;
  assert(ppcvm_pci_event_snapshot_encode(&s,buf,sizeof(buf))==0);
  /* Decode must leave the output untouched for every rejected frame. */
  decoded.version=99u;decoded.pending=99u;decoded.enable=99u;
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,15u)==-1);
  assert(decoded.version==99u && decoded.pending==99u && decoded.enable==99u);
  buf[0]='X';
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,sizeof(buf))==-1);
  buf[0]='P';buf[7]=2u;
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,sizeof(buf))==-1);
  buf[7]=1u;buf[11]=16u;
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,sizeof(buf))==-1);
  buf[11]=5u;buf[15]=16u;
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,sizeof(buf))==-1);
  assert(decoded.version==99u && decoded.pending==99u && decoded.enable==99u);
  assert(ppcvm_pci_event_snapshot_decode(&decoded,buf,17u)==-1);
  assert(ppcvm_pci_event_snapshot_decode(0,buf,sizeof(buf))==-1);
  assert(ppcvm_pci_event_snapshot_encode(0,buf,sizeof(buf))==-1);
  return 0;
}
