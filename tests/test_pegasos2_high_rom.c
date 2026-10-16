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
  /* Minimal firmware handoff: ROM branches to a guest entry in RAM.
   * This proves instruction fetch crosses the ROM/RAM bus boundary;
   * it does not yet implement Open Firmware or Linux boot parameters. */
  rom[0x100]=0x48; rom[0x101]=0x00;
  rom[0x102]=0x10; rom[0x103]=0x02; /* b 0x1000, absolute */
  assert(ppcvm_bus_write32be(&m.bus,UINT32_C(0x1000),
      UINT32_C(0x38600063))==PPCVM_BUS_OK); /* addi r3,r0,99 */
  assert(ppcvm_pegasos2_boot_reset_vector(&m)==PPCVM_OK);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.pc==UINT32_C(0x1000));
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==99u);
  assert(m.cpu.pc==UINT32_C(0x1004));
  /* Firmware bootstrap initializes diagnostic argument registers before
   * branching to RAM. This is deliberately not an Open Firmware ABI. */
  rom[0x100]=0x38; rom[0x101]=0x60;
  rom[0x102]=0x00; rom[0x103]=0x07; /* addi r3,r0,7 */
  rom[0x104]=0x38; rom[0x105]=0x80;
  rom[0x106]=0x00; rom[0x107]=0x09; /* addi r4,r0,9 */
  rom[0x108]=0x48; rom[0x109]=0x00;
  rom[0x10a]=0x10; rom[0x10b]=0x02; /* b 0x1000, absolute */
  assert(ppcvm_bus_write32be(&m.bus,UINT32_C(0x1000),
      UINT32_C(0x7ca32214))==PPCVM_BUS_OK); /* add r5,r3,r4 */
  assert(ppcvm_pegasos2_boot_reset_vector(&m)==PPCVM_OK);
  assert(ppcvm_pegasos2_run(&m,4u,0)==PPCVM_OK);
  assert(m.cpu.pc==UINT32_C(0x1004));
  assert(m.cpu.gpr[3]==7u && m.cpu.gpr[4]==9u);
  assert(m.cpu.gpr[5]==16u);
  /* Diagnostic boot-info handoff: ROM passes a RAM pointer in r3;
   * guest code loads a big-endian word from the pointed-to record. */
  rom[0x100]=0x38; rom[0x101]=0x60;
  rom[0x102]=0x20; rom[0x103]=0x00; /* addi r3,r0,0x2000 */
  rom[0x104]=0x48; rom[0x105]=0x00;
  rom[0x106]=0x10; rom[0x107]=0x02; /* b 0x1000, absolute */
  assert(ppcvm_bus_write32be(&m.bus,UINT32_C(0x2000),
      UINT32_C(0x50564331))==PPCVM_BUS_OK); /* PVC1 marker */
  assert(ppcvm_bus_write32be(&m.bus,UINT32_C(0x1000),
      UINT32_C(0x80830000))==PPCVM_BUS_OK); /* lwz r4,0(r3) */
  assert(ppcvm_pegasos2_boot_reset_vector(&m)==PPCVM_OK);
  assert(ppcvm_pegasos2_run(&m,3u,0)==PPCVM_OK);
  assert(m.cpu.pc==UINT32_C(0x1004));
  assert(m.cpu.gpr[3]==UINT32_C(0x2000));
  assert(m.cpu.gpr[4]==PPCVM_PEGASOS2_BOOT_MAGIC);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
