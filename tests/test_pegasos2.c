#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define D(op,rt,ra,disp) (((uint32_t)(op)<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(disp)&0xffffu))
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t value=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(m.bus.count==2 && m.cpu.pc==0);
  assert(ppcvm_bus_write32be(&m.bus,4,0x12345678u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&m.bus,4,&value)==PPCVM_BUS_OK && value==0x12345678u);
  m.cpu.gpr[1]=PPCVM_PEGASOS2_DISCOVERY_BASE;
  m.cpu.gpr[3]=0xaabbccddu;
  assert(ppcvm_cpu_step_bus(&m.cpu,&m.bus,D(36,3,1,0))==PPCVM_OK);
  assert(ppcvm_cpu_step_bus(&m.cpu,&m.bus,D(32,4,1,0))==PPCVM_OK);
  assert(m.cpu.gpr[4]==0xaabbccddu);
  assert(m.discovery_reads==1 && m.discovery_writes==1);
  assert(ppcvm_bus_read32be(&m.bus,PPCVM_PEGASOS2_DISCOVERY_BASE+4,&value)==PPCVM_BUS_INVALID);
  ppcvm_pegasos2_destroy(&m);
  assert(m.ram.data==0 && m.bus.count==0);
  assert(ppcvm_pegasos2_init(&m,0)==-1);
  return 0;
}
