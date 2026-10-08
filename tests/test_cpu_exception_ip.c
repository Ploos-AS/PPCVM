#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  cpu.msr=UINT32_C(0x0000c070); /* EE PR IR DR IP */
  ppcvm_cpu_enter_exception(&cpu,PPCVM_VECTOR_DSI,UINT32_C(0x1234));
  assert(cpu.pc==UINT32_C(0xfff00300));
  assert(cpu.srr0==UINT32_C(0x1234));
  assert(cpu.srr1==UINT32_C(0x0000c070));
  assert(cpu.msr==UINT32_C(0x40)); /* IP retained, execution modes cleared */
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(cpu.pc==UINT32_C(0x1234));
  assert(cpu.msr==UINT32_C(0x0000c070));
  ppcvm_cpu_enter_exception(&cpu,PPCVM_VECTOR_ISI,UINT32_C(0x2000));
  assert(cpu.pc==UINT32_C(0xfff00400));
  cpu.msr=0;
  ppcvm_cpu_enter_exception(&cpu,PPCVM_VECTOR_PROGRAM,UINT32_C(0x3000));
  assert(cpu.pc==PPCVM_VECTOR_PROGRAM);
  return 0;
}
