#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
#define D(op,rt,ra,disp) (((uint32_t)(op)<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(disp)&0xffffu))
int main(void) {
  ppcvm_bus b;
  ppcvm_cpu c;
  uint8_t ram[64]={0}, rom[4]={0};
  ppcvm_bus_init(&b);
  assert(ppcvm_bus_map_memory(&b,0,64,ram,0)==PPCVM_BUS_OK);
  assert(ppcvm_bus_map_memory(&b,0x100,4,rom,1)==PPCVM_BUS_OK);
  ppcvm_cpu_reset(&c);
  c.gpr[1]=16; c.gpr[3]=0x12345678u;
  assert(ppcvm_cpu_step_bus(&c,&b,D(36,3,1,4))==PPCVM_OK);
  assert(ppcvm_cpu_step_bus(&c,&b,D(32,4,1,4))==PPCVM_OK);
  assert(c.gpr[4]==0x12345678u && c.pc==8);
  assert(ppcvm_cpu_step_bus(&c,&b,D(34,5,1,5))==PPCVM_OK);
  assert(c.gpr[5]==0x34u);
  c.gpr[6]=0xa5u;
  assert(ppcvm_cpu_step_bus(&c,&b,D(38,6,1,7))==PPCVM_OK);
  assert(ram[23]==0xa5u);
  uint32_t old_pc=c.pc;
  assert(ppcvm_cpu_step_bus(&c,&b,D(36,3,0,0x100))==PPCVM_MEMORY_FAULT);
  assert(c.pc==old_pc);
  assert(ppcvm_cpu_step_bus(&c,&b,D(32,4,1,3))==PPCVM_MEMORY_FAULT);
  assert(c.pc==old_pc);
  assert(ppcvm_cpu_step_bus(&c,&b,D(14,7,0,42))==PPCVM_OK);
  assert(c.gpr[7]==42 && c.pc==old_pc+4);
  return 0;
}
