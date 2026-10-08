#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t value=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x44000002)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x200,PPCVM_PEGASOS2_FW_QUERY_RAM_BYTES)==PPCVM_MEM_OK);
  m.cpu.pc=0x100;m.cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;m.cpu.gpr[4]=0x200;
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x104 && m.cpu.gpr[3]==0);
  assert(ppcvm_memory_read32be(&m.ram,0x208,&value)==PPCVM_MEM_OK && value==65536);
  assert(ppcvm_memory_read32be(&m.ram,0x20c,&value)==PPCVM_MEM_OK && value==0);
  m.cpu.pc=0x100;m.cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;m.cpu.gpr[4]=0x201;
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_MEMORY_FAULT);
  assert(m.cpu.pc==0x100 && m.cpu.gpr[3]==PPCVM_PEGASOS2_BOOT_MAGIC);
  assert(ppcvm_memory_write32be(&m.ram,0x200,999)==PPCVM_MEM_OK);
  m.cpu.gpr[4]=0x200;
  assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x104 && m.cpu.gpr[3]==1);
  assert(ppcvm_memory_read32be(&m.ram,0x20c,&value)==PPCVM_MEM_OK && value==1);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
