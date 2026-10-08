#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.cpu.pc=0x2000; m.cpu.msr=UINT32_C(0x0000c030);
  assert(ppcvm_pegasos2_step_isi(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_ISI && m.cpu.srr0==0x2000);
  assert(m.cpu.srr1==UINT32_C(0x0000c030) && m.cpu.msr==0);
  m.cpu.pc=0x2001;
  assert(ppcvm_pegasos2_step_isi(&m)==PPCVM_MEMORY_FAULT && m.cpu.pc==0x2001);
  m.cpu.pc=0x2000;
  assert(ppcvm_pegasos2_step(&m)==PPCVM_MEMORY_FAULT && m.cpu.pc==0x2000);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
