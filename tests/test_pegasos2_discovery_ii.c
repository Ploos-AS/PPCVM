#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t value=0xdeadbeefu;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(m.bus.count==2);
  assert(ppcvm_pegasos2_map_discovery_ii(&m,0xf2000000u,4096)==PPCVM_BUS_OK);
  assert(m.bus.count==3);
  /* Legacy synthetic scratch remains isolated and operational. */
  assert(ppcvm_bus_write32be(&m.bus,PPCVM_PEGASOS2_DISCOVERY_BASE,0x12345678u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&m.bus,PPCVM_PEGASOS2_DISCOVERY_BASE,&value)==PPCVM_BUS_OK);
  assert(value==0x12345678u);
  /* No real Discovery II register offsets have been enabled. */
  value=0xdeadbeefu;
  assert(ppcvm_bus_read32be(&m.bus,0xf2000000u,&value)==PPCVM_BUS_UNMAPPED);
  assert(value==0xdeadbeefu);
  assert(m.discovery_ii.unsupported_reads==1);
  assert(ppcvm_bus_write32be(&m.bus,0xf2000004u,0xaabbccddu)==PPCVM_BUS_UNMAPPED);
  assert(m.discovery_ii.unsupported_writes==1);
  ppcvm_pci_bus pci;
  ppcvm_pci_device device;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&device,0x1234,0x5678,0,0,0);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&device)==0);
  ppcvm_discovery_ii_enable_pci_config(&m.discovery_ii,&pci,0);
  assert(ppcvm_bus_write32be(&m.bus,0xf2000cf8u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&m.bus,0xf2000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0x34127856u);
  ppcvm_pegasos2_reset(&m);
  assert(m.discovery_ii.config_address[0]==0u);
  assert(m.discovery_ii.pci_config_enabled==1u && m.discovery_ii.pci[0]==&pci);
  assert(ppcvm_bus_read32be(&m.bus,0xf2000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(ppcvm_bus_write32be(&m.bus,0xf2000cf8u,0x00100080u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&m.bus,0xf2000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==0x34127856u);
  assert(m.discovery_ii.reset_count==1);
  assert(m.discovery_ii.unsupported_reads==0 && m.discovery_ii.unsupported_writes==0);
  assert(m.discovery_scratch==0);
  assert(m.bus.count==3);
  ppcvm_pegasos2_cold_reset(&m);
  assert(m.discovery_ii.reset_count==2);
  assert(m.discovery_ii.config_address[0]==0u);
  assert(m.discovery_ii.pci[0]==&pci);
  assert(ppcvm_bus_read32be(&m.bus,0xf2000cfcu,&value)==PPCVM_BUS_OK);
  assert(value==UINT32_MAX);
  assert(m.bus.count==3);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
