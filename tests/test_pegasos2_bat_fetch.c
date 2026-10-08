#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x38600007))==PPCVM_MEM_OK);
  m.bat.ibatu[0]=UINT32_C(0x80000002);
  m.bat.ibatl[0]=0;
  m.cpu.pc=UINT32_C(0x80000000);
  m.cpu.msr=UINT32_C(0x20);
  assert(ppcvm_pegasos2_step_bat_fetch(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==7 && m.cpu.pc==UINT32_C(0x80000004));
  m.cpu.pc=UINT32_C(0x90000000);
  assert(ppcvm_pegasos2_step_bat_fetch(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_ISI && m.cpu.srr0==UINT32_C(0x90000000));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
