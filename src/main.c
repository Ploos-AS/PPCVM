#include "ppcvm/cpu.h"
#include <stdio.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  puts("PPCVM M0 - CPU scaffold (no guest boot support)");
  return cpu.pc != 0;
}
