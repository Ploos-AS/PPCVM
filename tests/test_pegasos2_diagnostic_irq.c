#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,65536u)==0);
  /* ori r0,r0,0 is a supported no-op; rfi is exception return. */
  assert(ppcvm_memory_write32be(&m.ram,0x1000u,UINT32_C(0x60000000))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x1004u,UINT32_C(0x60000000))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,PPCVM_VECTOR_EXTERNAL,UINT32_C(0x4c000064))==PPCVM_MEM_OK);
  m.cpu.pc=0x1000u;
  m.cpu.msr=UINT32_C(0x8000);
  ppcvm_irq_latch_set(&m.diagnostic_irq,1u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u); /* source masked */
  m.cpu.pc=0x1000u;
  ppcvm_irq_latch_enable(&m.diagnostic_irq,1u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL);
  assert(m.cpu.srr0==0x1000u && m.cpu.srr1==UINT32_C(0x8000));
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK); /* rfi */
  assert(m.cpu.pc==0x1000u && (m.cpu.msr&UINT32_C(0x8000)));
  ppcvm_irq_latch_clear(&m.diagnostic_irq,1u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u);
  /* A still-asserted level retriggers after rfi until acknowledged. */
  ppcvm_irq_latch_set(&m.diagnostic_irq,1u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL && m.cpu.srr0==0x1004u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL); /* level still pending */
  ppcvm_irq_latch_clear(&m.diagnostic_irq,1u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u);
  /* A second source is independent of source one. */
  ppcvm_irq_latch_set(&m.diagnostic_irq,2u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1008u); /* second source masked */
  ppcvm_irq_latch_enable(&m.diagnostic_irq,2u);
  assert(ppcvm_pegasos2_step_diagnostic_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL && m.cpu.srr0==0x1008u);
  ppcvm_pegasos2_reset(&m);
  assert(ppcvm_irq_latch_active(&m.diagnostic_irq)==0);
  assert(m.diagnostic_irq.asserted==0 && m.diagnostic_irq.enabled==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
