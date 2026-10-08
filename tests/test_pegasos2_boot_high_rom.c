#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint8_t rom[4096];
  memset(rom,0,sizeof(rom));
  /* addi r3,r0,42; addi r4,r3,1 */
  rom[0x100]=0x38; rom[0x101]=0x60; rom[0x102]=0x00; rom[0x103]=0x2a;
  rom[0x104]=0x38; rom[0x105]=0x83; rom[0x106]=0x00; rom[0x107]=0x01;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_pegasos2_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_MEMORY_FAULT);
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_OK);
  m.cpu.gpr[3]=99;
  assert(ppcvm_pegasos2_boot_high_rom(&m,UINT32_C(0xfff00101))==PPCVM_MEMORY_FAULT);
  assert(m.cpu.gpr[3]==99); /* failed boot is non-destructive */
  assert(ppcvm_pegasos2_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_OK);
  assert(m.cpu.gpr[3]==0);
  assert(m.cpu.pc==UINT32_C(0xfff00100));
  assert(m.cpu.msr==UINT32_C(0x40));
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[4]==43);
  assert(m.cpu.pc==UINT32_C(0xfff00108));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
