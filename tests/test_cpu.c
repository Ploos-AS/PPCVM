#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define I(op,rs,ra,imm) (((uint32_t)(op)<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
int main(void) {
  ppcvm_cpu c;
  ppcvm_cpu_reset(&c);
  assert(c.pc == 0 && c.lr == 0 && c.cr == 0);
  assert(ppcvm_cpu_step(&c, I(14,3,0,0xffff)) == PPCVM_OK);
  assert(c.gpr[3] == UINT32_MAX && c.pc == 4);
  assert(ppcvm_cpu_step(&c, I(24,3,4,0xff)) == PPCVM_OK);
  assert(c.gpr[4] == UINT32_MAX && c.pc == 8);
  assert(ppcvm_cpu_step(&c, I(15,5,0,0x8000)) == PPCVM_OK);
  assert(c.gpr[5] == 0x80000000u);
  assert(ppcvm_cpu_step(&c, I(25,0,6,0x1234)) == PPCVM_OK);
  assert(c.gpr[6] == 0x12340000u);
  assert(ppcvm_cpu_step(&c, I(26,6,7,0x00ff)) == PPCVM_OK);
  assert(c.gpr[7] == 0x123400ffu);
  assert(ppcvm_cpu_step(&c, I(28,7,8,0)) == PPCVM_OK);
  assert(c.gpr[8] == 0 && (c.cr >> 28) == 2);
  c.pc = 0x100;
  assert(ppcvm_cpu_step(&c, (18u<<26)|0xfffffffcu) == PPCVM_OK);
  assert(c.pc == 0xfc);
  assert(ppcvm_cpu_step(&c, (18u<<26)|8u|1u) == PPCVM_OK);
  assert(c.lr == 0x100 && c.pc == 0x104);
  assert(ppcvm_cpu_step(&c, 0xffffffffu) == PPCVM_UNSUPPORTED);
  assert(c.pc == 0x104);
  return 0;
}
