#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t v=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  /* Guest instructions:
     addi r5,r0,2 ; stw r5,0(r4) ; sc ; lwz r7,8(r4) */
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x38a00002)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x104,0x90a40000)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x108,0x44000002)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x10c,0x80e40008)==PPCVM_MEM_OK);
  m.cpu.pc=0x100;
  m.cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;
  m.cpu.gpr[4]=0x200;
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.gpr[5]==2 && m.cpu.pc==0x104);
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(ppcvm_memory_read32be(&m.ram,0x200,&v)==PPCVM_MEM_OK && v==2);
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==0 && m.cpu.pc==0x10c);
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.gpr[7]==65536 && m.cpu.pc==0x110);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
