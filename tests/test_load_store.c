#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define D(op,rt,ra,disp) (((uint32_t)(op)<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(disp)&0xffffu))
int main(void) {
  ppcvm_memory m = {0};
  ppcvm_cpu c;
  assert(ppcvm_memory_init(&m, 64) == PPCVM_MEM_OK);
  ppcvm_cpu_reset(&c);
  c.gpr[1] = 16;
  c.gpr[3] = 0x12345678u;
  assert(ppcvm_cpu_step_memory(&c, &m, D(36,3,1,4)) == PPCVM_OK);
  assert(c.pc == 4 && m.data[20] == 0x12 && m.data[23] == 0x78);
  assert(ppcvm_cpu_step_memory(&c, &m, D(32,4,1,4)) == PPCVM_OK);
  assert(c.gpr[4] == 0x12345678u);
  assert(ppcvm_cpu_step_memory(&c, &m, D(34,5,1,5)) == PPCVM_OK);
  assert(c.gpr[5] == 0x34u);
  c.gpr[6] = 0xabcdef9au;
  assert(ppcvm_cpu_step_memory(&c, &m, D(38,6,1,7)) == PPCVM_OK);
  assert(m.data[23] == 0x9au);
  assert(ppcvm_cpu_step_memory(&c, &m, D(32,7,1,4)) == PPCVM_OK);
  assert(c.gpr[7] == 0x1234569au);
  uint32_t saved_pc = c.pc;
  c.gpr[8] = 0xa5a5a5a5u;
  assert(ppcvm_cpu_step_memory(&c, &m, D(32,8,1,3)) == PPCVM_MEMORY_FAULT);
  assert(c.pc == saved_pc && c.gpr[8] == 0xa5a5a5a5u);
  assert(ppcvm_cpu_step_memory(&c, &m, D(36,3,1,48)) == PPCVM_MEMORY_FAULT);
  assert(c.pc == saved_pc);
  assert(ppcvm_cpu_step(&c, D(32,8,1,4)) == PPCVM_MEMORY_FAULT);
  assert(c.pc == saved_pc);
  ppcvm_memory_free(&m);
  return 0;
}
