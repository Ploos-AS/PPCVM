#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define LWZ ((32u<<26)|(3u<<21)|(4u<<16))
#define STW ((36u<<26)|(3u<<21)|(4u<<16))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.bat.ibatu[0]=UINT32_C(0x80000002);
  m.bat.ibatl[0]=0;
  m.bat.dbatu[0]=UINT32_C(0x90000002);
  m.bat.dbatl[0]=0;
  m.cpu.pc=UINT32_C(0x80000000);
  m.cpu.msr=UINT32_C(0x30);
  m.cpu.gpr[4]=UINT32_C(0x90000100);
  assert(ppcvm_memory_write32be(&m.ram,0,STW)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,4,LWZ)==PPCVM_MEM_OK);
  m.cpu.gpr[3]=UINT32_C(0x12345678);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  m.cpu.gpr[3]=0;
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==UINT32_C(0x12345678));
  m.cpu.gpr[4]=UINT32_C(0xa0000100);
  m.cpu.pc=UINT32_C(0x80000004);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI && m.cpu.dar==UINT32_C(0xa0000100));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
