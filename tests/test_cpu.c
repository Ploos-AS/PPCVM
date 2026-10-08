#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  assert(cpu.pc == 0 && cpu.gpr[3] == 0);
  /* addi r3,r0,-1 */
  assert(ppcvm_cpu_step(&cpu, (14u << 26) | (3u << 21) | 0xffffu) == PPCVM_OK);
  assert(cpu.gpr[3] == UINT32_MAX && cpu.pc == 4);
  /* ori r4,r3,0xff */
  assert(ppcvm_cpu_step(&cpu, (24u << 26) | (3u << 21) | (4u << 16) | 0xffu) == PPCVM_OK);
  assert(cpu.gpr[4] == UINT32_MAX && cpu.pc == 8);
  /* unsupported instruction must not advance PC */
  assert(ppcvm_cpu_step(&cpu, 0xffffffffu) == PPCVM_UNSUPPORTED);
  assert(cpu.pc == 8);
  return 0;
}
