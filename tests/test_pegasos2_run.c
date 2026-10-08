#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define ADDI(rt,ra,imm) ((14u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
int main(void) {
  ppcvm_pegasos2 m;
  size_t executed=999;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  assert(ppcvm_bus_write32be(&m.bus,0,ADDI(3,0,10))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,4,ADDI(3,3,1))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,8,0xffffffffu)==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_run(&m,0,&executed)==PPCVM_OK && executed==0);
  assert(ppcvm_pegasos2_run(&m,1,&executed)==PPCVM_OK && executed==1);
  assert(m.cpu.gpr[3]==10 && m.cpu.pc==4);
  assert(ppcvm_pegasos2_run(&m,10,&executed)==PPCVM_UNSUPPORTED && executed==1);
  assert(m.cpu.gpr[3]==11 && m.cpu.pc==8);
  assert(ppcvm_pegasos2_run(&m,10,&executed)==PPCVM_UNSUPPORTED && executed==0);
  assert(m.cpu.pc==8);
  assert(ppcvm_pegasos2_run(0,1,&executed)==PPCVM_MEMORY_FAULT && executed==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
