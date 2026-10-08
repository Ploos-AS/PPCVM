#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define LWZ ((32u<<26)|(3u<<21)|(4u<<16))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.cpu.pc=0x2000; m.cpu.msr=UINT32_C(0x0000c030);
  assert(ppcvm_pegasos2_step_exceptions(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_ISI && m.cpu.srr0==0x2000);
  ppcvm_pegasos2_destroy(&m);
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  assert(ppcvm_memory_write32be(&m.ram,0,LWZ)==PPCVM_MEM_OK);
  m.cpu.gpr[4]=0x2000; m.cpu.msr=UINT32_C(0x0000c030);
  assert(ppcvm_pegasos2_step_exceptions(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI && m.cpu.srr0==0);
  assert(m.cpu.dar==0x2000 && m.cpu.dsisr==UINT32_C(0x40000000));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
