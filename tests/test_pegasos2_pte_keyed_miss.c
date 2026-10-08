#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x90640000))==PPCVM_MEM_OK); /* stw r3,0(r4) */
  m.segments.sr[0]=UINT32_C(0x20000123);
  m.cpu.msr=UINT32_C(0x4010);
  m.cpu.gpr[3]=UINT32_C(0x11223344);
  m.cpu.gpr[4]=UINT32_C(0x4000);
  /* No PTE: translation miss must not be reported as protection. */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.dar==UINT32_C(0x4000));
  assert(m.cpu.dsisr==UINT32_C(0x42000000)); /* store + translation miss */
  assert(m.cpu.srr0==0);
  ppcvm_pegasos2_destroy(&m);
  assert(ppcvm_pegasos2_init(&m,131072)==0);
  m.segments.sr[0]=UINT32_C(0x20000123);
  m.cpu.msr=UINT32_C(0x4020);
  /* Missing instruction PTE must raise ISI without advancing PC. */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_ISI);
  assert(m.cpu.srr0==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
