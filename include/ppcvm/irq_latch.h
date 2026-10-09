#ifndef PPCVM_IRQ_LATCH_H
#define PPCVM_IRQ_LATCH_H
#include <stdint.h>
/* Synthetic test primitive, NOT a Discovery II register model. */
typedef struct {
  uint32_t asserted;
  uint32_t enabled;
} ppcvm_irq_latch;
static inline void ppcvm_irq_latch_reset(ppcvm_irq_latch *l) {
  if (!l) return;
  l->asserted = 0;
  l->enabled = 0;
}
static inline void ppcvm_irq_latch_set(ppcvm_irq_latch *l, uint32_t bits) {
  if (l) l->asserted |= bits;
}
static inline void ppcvm_irq_latch_clear(ppcvm_irq_latch *l, uint32_t bits) {
  if (l) l->asserted &= ~bits;
}
static inline void ppcvm_irq_latch_enable(ppcvm_irq_latch *l, uint32_t bits) {
  if (l) l->enabled |= bits;
}
static inline void ppcvm_irq_latch_disable(ppcvm_irq_latch *l, uint32_t bits) {
  if (l) l->enabled &= ~bits;
}
static inline uint32_t ppcvm_irq_latch_active(const ppcvm_irq_latch *l) {
  return l ? (l->asserted & l->enabled) : 0;
}
#endif
