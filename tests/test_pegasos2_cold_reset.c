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
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_OK);
  assert(ppcvm_memory_write32be(&m.ram,0,UINT32_C(0x12345678))==PPCVM_MEM_OK);
  m.cpu.gpr[5]=77;
  m.bat.ibatu[0]=UINT32_C(0xdeadbeef);
  m.segments.sr[0]=UINT32_C(0x123);
  m.segments.sdr1=UINT32_C(0x1000);
  m.discovery_scratch=88;
  m.discovery_reads=3;
  m.discovery_writes=4;
  size_t count=m.bus.count;
  ppcvm_pegasos2_cold_reset(&m);
  assert(m.cpu.gpr[5]==0 && m.cpu.pc==0 && m.cpu.msr==0);
  assert(m.discovery_scratch==0 && m.discovery_reads==0 && m.discovery_writes==0);
  assert(m.bus.count==count);
  assert(m.bat.ibatu[0]==0 && m.segments.sr[0]==0 && m.segments.sdr1==0);
  uint32_t word=UINT32_C(0xffffffff);
  assert(ppcvm_memory_read32be(&m.ram,0,&word)==PPCVM_MEM_OK);
  assert(word==0);
  assert(ppcvm_pegasos2_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_OK);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
