#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define LWZ ((32u<<26)|(3u<<21)|(4u<<16))
#define STW ((36u<<26)|(3u<<21)|(4u<<16))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  m.bat.dbatu[0]=UINT32_C(0x90000002);
  m.bat.dbatl[0]=0; /* PP=00 */
  m.cpu.msr=UINT32_C(0x10);
  m.cpu.gpr[4]=UINT32_C(0x90000100);
  assert(ppcvm_memory_write32be(&m.ram,0,LWZ)==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI && m.cpu.dar==UINT32_C(0x90000100));
  assert(m.cpu.dsisr==UINT32_C(0x08000000));
  m.cpu.pc=0;
  m.cpu.msr=UINT32_C(0x10);
  assert(ppcvm_memory_write32be(&m.ram,0,STW)==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_bat(&m)==PPCVM_OK);
  assert(m.cpu.dsisr==UINT32_C(0x0a000000));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
