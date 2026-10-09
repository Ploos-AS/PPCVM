#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_pegasos2 m;
  const uint32_t base=UINT32_C(0x20000);
  assert(ppcvm_pegasos2_init(&m,65536u)==0);
  assert(ppcvm_pegasos2_map_discovery_ii(&m,base,0x100u)==PPCVM_BUS_OK);
  ppcvm_discovery_ii_enable_irq_candidate(&m.discovery_ii);
  /* Handler: stw r4,0x14(r3); rfi. The source remains asserted but
     masking it in the controller prevents another external exception. */
  assert(ppcvm_memory_write32be(&m.ram,PPCVM_VECTOR_EXTERNAL,UINT32_C(0x90830014))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,PPCVM_VECTOR_EXTERNAL+4u,UINT32_C(0x4c000064))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x1000u,UINT32_C(0x60000000))==PPCVM_MEM_OK);
  m.cpu.pc=0x1000u;
  m.cpu.msr=UINT32_C(0x8000);
  m.cpu.gpr[3]=base;
  m.cpu.gpr[4]=0;
  ppcvm_discovery_ii_assert_irq_low(&m.discovery_ii,1u);
  assert(ppcvm_bus_write32be(&m.bus,base+PPCVM_DISCOVERY_II_IRQ_CPU0_MASK_LOW_CANDIDATE,ppcvm_discovery_ii_swap32(1u))==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL && m.cpu.srr0==0x1000u);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_EXTERNAL+4u);
  assert(m.discovery_ii.irq_asserted_low==1u);
  assert(ppcvm_discovery_ii_active_irq_low(&m.discovery_ii)==0);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1000u);
  assert(ppcvm_pegasos2_step_discovery_irq(&m)==PPCVM_OK);
  assert(m.cpu.pc==0x1004u);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
