#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  const uint8_t program[]={0x38,0x60,0x00,0x2a,0x38,0x83,0x00,0x01};
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_pegasos2_load_raw(&m,0x100,program,sizeof(program))==PPCVM_OK);
  assert(ppcvm_pegasos2_load_raw(&m,65532,program,sizeof(program))==PPCVM_MEMORY_FAULT);
  assert(ppcvm_pegasos2_load_raw(&m,0,program,0)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_pegasos2_enter_ram(&m,0x101)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_pegasos2_enter_ram(&m,65536)==PPCVM_MEMORY_FAULT);
  assert(m.cpu.pc==0);
  assert(ppcvm_pegasos2_enter_ram(&m,0x100)==PPCVM_OK);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42 && m.cpu.gpr[4]==43 && m.cpu.pc==0x108);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
