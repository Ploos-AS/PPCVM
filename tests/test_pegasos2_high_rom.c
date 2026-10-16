#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
int main(void) {
  ppcvm_pegasos2 m;
  uint8_t rom[4096];
  memset(rom,0,sizeof(rom));
  /* rfi at the DSI high vector offset 0x300. */
  rom[0x300]=0x4c; rom[0x301]=0x00; rom[0x302]=0x00; rom[0x303]=0x64;
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_INVALID);
  uint32_t word=0;
  assert(ppcvm_bus_read32be(&m.bus,UINT32_C(0xfff00300),&word)==PPCVM_BUS_OK);
  assert(word==UINT32_C(0x4c000064));
  assert(ppcvm_bus_write32be(&m.bus,UINT32_C(0xfff00300),0)==PPCVM_BUS_READ_ONLY);
  m.cpu.msr=UINT32_C(0x40);
  ppcvm_cpu_enter_exception(&m.cpu,PPCVM_VECTOR_DSI,UINT32_C(0x100));
  assert(m.cpu.pc==UINT32_C(0xfff00300));
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.pc==UINT32_C(0x100));
  assert(m.cpu.msr==UINT32_C(0x40));
  /* Reset-vector boot must start at the high prefix, not at RAM zero. */
  rom[0x100]=0x38; rom[0x101]=0x60;
  rom[0x102]=0x00; rom[0x103]=0x2a; /* addi r3,r0,42 */
  m.cpu.gpr[3]=99u;
  assert(ppcvm_pegasos2_boot_reset_vector(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_PEGASOS2_RESET_VECTOR);
  assert(m.cpu.msr==UINT32_C(0x40));
  assert(m.cpu.gpr[3]==0u);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42u);
  assert(m.cpu.pc==PPCVM_PEGASOS2_RESET_VECTOR+4u);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
