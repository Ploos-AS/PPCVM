#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
int main(void) {
  ppcvm_cpu cpu;
  ppcvm_cpu_reset(&cpu);
  cpu.msr=UINT32_C(0x0000c030); /* EE PR IR DR */
  cpu.pc=UINT32_C(0x1234);
  ppcvm_cpu_enter_exception(&cpu,PPCVM_VECTOR_DSI,cpu.pc);
  assert(cpu.pc==PPCVM_VECTOR_DSI);
  assert(cpu.srr0==UINT32_C(0x1234));
  assert(cpu.srr1==UINT32_C(0x0000c030));
  assert((cpu.msr&UINT32_C(0x0000c030))==0);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK); /* rfi */
  assert(cpu.pc==UINT32_C(0x1234));
  assert(cpu.msr==UINT32_C(0x0000c030));
  /* Return target must be word aligned. */
  cpu.msr=0;
  cpu.srr0=UINT32_C(0x1237);
  cpu.srr1=UINT32_C(0x4000);
  assert(ppcvm_cpu_step(&cpu,UINT32_C(0x4c000064))==PPCVM_OK);
  assert(cpu.pc==UINT32_C(0x1234));
  assert(cpu.msr==UINT32_C(0x4000));
  return 0;
}
