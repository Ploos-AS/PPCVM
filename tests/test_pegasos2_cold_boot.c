#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint8_t rom[4096];
  memset(rom,0,sizeof(rom));
  rom[0x100]=0x38; rom[0x101]=0x60; rom[0x102]=0x00; rom[0x103]=0x2a;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0xdeadbeef))==PPCVM_MEM_OK);
  m.cpu.gpr[7]=77;
  assert(ppcvm_pegasos2_cold_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_MEMORY_FAULT);
  assert(m.cpu.gpr[7]==77);
  uint32_t value=0;
  assert(ppcvm_memory_read32be(&m.ram,0,&value)==PPCVM_MEM_OK && value==UINT32_C(0xdeadbeef));
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_cold_boot_high_rom(&m,UINT32_C(0xfff00101))==PPCVM_MEMORY_FAULT);
  assert(m.cpu.gpr[7]==77);
  assert(ppcvm_pegasos2_cold_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_OK);
  assert(m.cpu.gpr[7]==0 && m.cpu.pc==UINT32_C(0xfff00100) && m.cpu.msr==UINT32_C(0x40));
  assert(ppcvm_memory_read32be(&m.ram,0,&value)==PPCVM_MEM_OK && value==0);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
