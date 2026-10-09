#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,65536u)==0);
  assert(ppcvm_memory_write32be(&m.ram,0x1000u,UINT32_C(0x60000000))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x1004u,UINT32_C(0x60000000))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,PPCVM_VECTOR_EXTERNAL,UINT32_C(0x4c000064))==PPCVM_MEM_OK);
  m.cpu.pc=0x1000u;
  m.cpu.msr=UINT32_C(0x8000);
  ppcvm_discovery_ii_enable_irq_candidate(&m.discovery_ii);
  ppcvm_discovery_ii_assert_irq_low(&m.discovery_ii,1u);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u); /* masked in controller */
  m.cpu.msr=0;
  m.discovery_ii.irq_cpu0_mask_low=1u;
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1008u); /* masked in CPU */
  m.cpu.msr=UINT32_C(0x8000);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL && m.cpu.srr0==0x1008u);
  ppcvm_discovery_ii_clear_irq_low(&m.discovery_ii,1u);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1008u); /* rfi */
  ppcvm_pegasos2_reset(&m);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==0);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
