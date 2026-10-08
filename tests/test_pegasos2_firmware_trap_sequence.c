#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t v=0;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  /* Four consecutive synthetic firmware calls at the same sc site. */
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x44000002)==PPCVM_MEM_OK);
  m.cpu.gpr[4]=0x200;
  const uint32_t selectors[]={PPCVM_PEGASOS2_FW_QUERY_VERSION,999,
    PPCVM_PEGASOS2_FW_QUERY_RAM_BYTES,PPCVM_PEGASOS2_FW_QUERY_BOOT_MAGIC};
  const uint32_t expected[]={2,0,65536,PPCVM_PEGASOS2_BOOT_MAGIC};
  const uint32_t statuses[]={0,1,0,0};
  for (unsigned i=0;i<4;i++) {
    assert(ppcvm_memory_write32be(&m.ram,0x200,selectors[i])==PPCVM_MEM_OK);
    assert(ppcvm_memory_write32be(&m.ram,0x208,0xdeadbeef)==PPCVM_MEM_OK);
    assert(ppcvm_memory_write32be(&m.ram,0x20c,0xdeadbeef)==PPCVM_MEM_OK);
    m.cpu.pc=0x100;
    m.cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;
    assert(ppcvm_pegasos2_step_firmware(&m)==PPCVM_OK);
    assert(m.cpu.pc==0x104);
    assert(m.cpu.gpr[3]==statuses[i]);
    assert(ppcvm_memory_read32be(&m.ram,0x208,&v)==PPCVM_MEM_OK && v==expected[i]);
    assert(ppcvm_memory_read32be(&m.ram,0x20c,&v)==PPCVM_MEM_OK && v==statuses[i]);
  }
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
