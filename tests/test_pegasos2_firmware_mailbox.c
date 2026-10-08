#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t v=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_memory_write32be(&m.ram,0x200,PPCVM_PEGASOS2_FW_QUERY_RAM_BYTES)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x204,0x12345678)==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_firmware_mailbox(&m,0x200)==PPCVM_OK);
  assert(ppcvm_memory_read32be(&m.ram,0x208,&v)==PPCVM_MEM_OK && v==65536);
  assert(ppcvm_memory_read32be(&m.ram,0x20c,&v)==PPCVM_MEM_OK && v==0);
  assert(ppcvm_memory_read32be(&m.ram,0x204,&v)==PPCVM_MEM_OK && v==0x12345678);
  assert(ppcvm_memory_write32be(&m.ram,0x200,999)==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_firmware_mailbox(&m,0x200)==PPCVM_UNSUPPORTED);
  assert(ppcvm_memory_read32be(&m.ram,0x208,&v)==PPCVM_MEM_OK && v==0);
  assert(ppcvm_memory_read32be(&m.ram,0x20c,&v)==PPCVM_MEM_OK && v==1);
  assert(ppcvm_pegasos2_firmware_mailbox(&m,0x201)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_pegasos2_firmware_mailbox(&m,65524)==PPCVM_MEMORY_FAULT);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
