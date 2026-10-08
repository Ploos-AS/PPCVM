#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define D(op,rt,ra,imm) (((uint32_t)(op)<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  /* addi r3,r0,42 ; stw r3,32(r0) ; lwz r4,32(r0) ; b +8 ; skipped ; addi r5,r4,1 */
  assert(ppcvm_bus_write32be(&m.bus,0,D(14,3,0,42))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,4,D(36,3,0,32))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,8,D(32,4,0,32))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,12,(18u<<26)|8u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,16,D(14,5,0,99))==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&m.bus,20,D(14,5,4,1))==PPCVM_BUS_OK);
  for (unsigned i=0;i<5;i++) assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42 && m.cpu.gpr[4]==42 && m.cpu.gpr[5]==43);
  assert(m.cpu.pc==24);
  m.cpu.pc=4096;
  assert(ppcvm_pegasos2_step(&m)==PPCVM_MEMORY_FAULT && m.cpu.pc==4096);
  m.cpu.pc=1;
  assert(ppcvm_pegasos2_step(&m)==PPCVM_MEMORY_FAULT && m.cpu.pc==1);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
