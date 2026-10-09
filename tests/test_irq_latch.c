#include "ppcvm/irq_latch.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_irq_latch l = {0};
  assert(ppcvm_irq_latch_active(&l) == 0);
  ppcvm_irq_latch_set(&l, UINT32_C(0x5));
  assert(l.asserted == 5 && ppcvm_irq_latch_active(&l) == 0);
  ppcvm_irq_latch_enable(&l, UINT32_C(0x1));
  assert(ppcvm_irq_latch_active(&l) == 1);
  ppcvm_irq_latch_enable(&l, UINT32_C(0x4));
  assert(ppcvm_irq_latch_active(&l) == 5);
  ppcvm_irq_latch_disable(&l, UINT32_C(0x1));
  assert(l.asserted == 5 && ppcvm_irq_latch_active(&l) == 4);
  ppcvm_irq_latch_clear(&l, UINT32_C(0x4));
  assert(ppcvm_irq_latch_active(&l) == 0 && l.asserted == 1);
  ppcvm_irq_latch_enable(&l, UINT32_C(0x1));
  assert(ppcvm_irq_latch_active(&l) == 1);
  ppcvm_irq_latch_reset(&l);
  assert(l.asserted == 0 && l.enabled == 0 && ppcvm_irq_latch_active(&l) == 0);
  assert(ppcvm_irq_latch_active(0) == 0);
  return 0;
}
