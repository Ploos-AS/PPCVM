#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t v=0xdeadbeefu;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_pegasos2_firmware_query(&m,PPCVM_PEGASOS2_FW_QUERY_VERSION,&v)==PPCVM_OK && v==2);
  assert(ppcvm_pegasos2_firmware_query(&m,PPCVM_PEGASOS2_FW_QUERY_RAM_BYTES,&v)==PPCVM_OK && v==65536);
  assert(ppcvm_pegasos2_firmware_query(&m,PPCVM_PEGASOS2_FW_QUERY_BOOT_MAGIC,&v)==PPCVM_OK && v==PPCVM_PEGASOS2_BOOT_MAGIC);
  v=0xdeadbeefu;
  assert(ppcvm_pegasos2_firmware_query(&m,999,&v)==PPCVM_UNSUPPORTED && v==0xdeadbeefu);
  assert(ppcvm_pegasos2_firmware_query(&m,1,0)==PPCVM_MEMORY_FAULT);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
