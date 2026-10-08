#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t word=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x44000002)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x200,PPCVM_PEGASOS2_FW_QUERY_VERSION)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x208,0xaabbccdd)==PPCVM_MEM_OK);
  m.cpu.pc=0x100;
  m.cpu.gpr[3]=0x12345678; /* not PVC1 magic */
  m.cpu.gpr[4]=0x200;
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_SYSCALL);
  assert(m.cpu.srr0==0x104);
  assert(m.cpu.gpr[3]==0x12345678);
  assert(ppcvm_memory_read32be(&m.ram,0x208,&word)==PPCVM_MEM_OK && word==0xaabbccdd);
  /* The regular stepping API must also preserve the normal sc exception. */
  m.cpu.pc=0x100;
  m.cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_SYSCALL);
  assert(ppcvm_memory_read32be(&m.ram,0x208,&word)==PPCVM_MEM_OK && word==0xaabbccdd);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
