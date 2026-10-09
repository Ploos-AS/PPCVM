#ifndef PPCVM_IRQ_BRIDGE_H
#define PPCVM_IRQ_BRIDGE_H
#include "ppcvm/cpu.h"
#include "ppcvm/irq_latch.h"
/* Synthetic board test bridge. The latch remains asserted until the guest
 * device/controller explicitly clears it; this function never acknowledges. */
static inline int ppcvm_irq_bridge_poll(ppcvm_cpu *cpu,
                                         const ppcvm_irq_latch *latch) {
  if (!cpu || !latch) return -1;
  if (!ppcvm_irq_latch_active(latch)) return 0;
  return ppcvm_cpu_accept_external_irq(cpu);
}
#endif
